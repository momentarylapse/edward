//
// Created by michi on 10/1/25.
//

#include "ComponentPanel.h"
#include "CameraPanel.h"
#include "LightPanel.h"
#include "SolidBodyPanel.h"
#include "ModelRefPanel.h"
#include "EntityBasePanel.h"
#include "TerrainRefPanel.h"
#include "UserComponentPanel.h"
#include "../data/DataWorld.h"
#include <y/world/Camera.h>
#include <y/y/Entity.h>
#include <y/world/components/Animator.h>
#include <y/world/components/Collider.h>
#include <y/world/components/Skeleton.h>
#include <y/world/components/SolidBody.h>
#include <y/world/Light.h>
#include <y/world/Model.h>
#include <y/world/Terrain.h>

#include <lib/kaba/syntax/Class.h>
#include <lib/kaba/syntax/SyntaxTree.h>
#include <lib/kaba/Module.h>

#include <storage/Storage.h>
#include <storage/format/Format.h>
#include <stuff/PluginManager.h>
#include <lib/xhui/controls/ListView.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/TextureManager.h>
#include <y/helper/ResourceManager.h>


class DummyComponentPanel : public xhui::Panel {
public:
	DummyComponentPanel() : Panel("dummy-panel") {
		add_control("Label", "!italic,expandx,center\\no configuration", 0, 0, "message");
	}
};

class MaterialComponentPanel : public xhui::Panel {
public:
	explicit MaterialComponentPanel(DataWorld* _data, yrenderer::Material* m, const Path& filename, std::function<void(const ComplexPath&)> _f_save) : Panel("material-panel") {
		from_source(R"foodelim(
Dialog material-panel ''
	Grid ? ''
		Grid ? ''
			Label ? 'Filename'
			Grid ? ''
				Button filename '' disabled
				Button load-material 'L' noexpandx
				Button save-material 'S' noexpandx
		---|
		TabControl ? 'Color\\Textures\\Shader'
			Grid ? ''
				Label ? 'Albedo'
				ColorButton albedo ''
				---|
				Label ? 'Emission'
				ColorButton emission ''
				---|
				Label ? 'Roughness'
				SpinButton roughness '' range=0:100:1
				---|
				Label ? 'Metal'
				SpinButton metal '' range=0:100:1
			ListView textures 'a\\filename' nobar format=it noexpandy height=200
			Grid ? ''
				Label ? 'Shader'
				Grid ? ''
					Button shader '' disabled
					Button load-shader 'L' noexpandx
)foodelim");
		auto list = dynamic_cast<xhui::ListView*>(get_control("textures"));
		list->column_factories[0].f_create = [] (const string& id) {
			auto im = new xhui::Image(id, "");
			im->min_width_user = 32;
			im->min_height_user = 32;
			im->size_mode_x = SizeMode::Shrink;
			im->size_mode_y = SizeMode::Shrink;
			return im;
		};
		data = _data;
		material = m;
		f_save = _f_save;
		set_string("filename", str(filename));
		set_string("shader", str(material->pass0.shader_path));
		set_color("albedo", material->albedo);
		set_color("emission", material->emission);
		set_float("roughness", material->roughness * 100);
		set_float("metal", material->metal * 100);
		fill_texture_list();

		event("load-material", [this] {

		});
		event("save-material", [this] {
			data->session->storage->file_dialog(FD_MATERIAL, true, true).then(f_save);
		});
		event("load-shader", [this] {
			data->session->storage->file_dialog(FD_SHADERFILE, false, true).then([this] (const auto& filename) {
				set_string("shader", str(filename.relative));
				material->pass0.shader_path = filename.relative;
			});
		});
		event("albedo", [this] {
			material->albedo = get_color("albedo");
		});
		event("emission", [this] {
			material->emission = get_color("emission");
		});
		event("roughness", [this] {
			material->roughness = get_float("roughness");
		});
		event("metal", [this] {
			material->metal = get_float("metal");
		});

		event("textures", [this] {
			int i = get_int("textures");
			if (i >= 0)
				data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, i] (const auto& filename) {
					material->textures[i] = data->session->resource_manager->load_texture(filename.relative);
					fill_texture_list();
					// TODO save by action
				});
		});
	}
	void fill_texture_list() {
		reset("textures");
		const Path dir = data->session->storage->get_root_dir(FD_TEXTURE);
		for (int i=0; i<min(MATERIAL_MAX_TEXTURES, material->textures.num); i++)
			add_string("textures", format("%s\\%s", xhui::texture_to_image(material->textures[i]), data->session->ctx->texture_manager->texture_file(material->textures[i].get()).relative_to(dir)));
	}
	yrenderer::Material* material;
	DataWorld* data;
	std::function<void(const ComplexPath&)> f_save;
};

class UnknownComponentPanel : public xhui::Panel {
public:
	explicit UnknownComponentPanel(DataWorld* _data, int _index, int _cindex) : Panel("unknown-component-panel") {
		from_source(R"foodelim(
Dialog unknown-component-panel ''
	Grid grid-variables ''
)foodelim");
		data = _data;
		index = _index;
		cindex = _cindex;

		auto e = data->entity(index);
		auto& cc = e->get_component<EdwardTag>()->unknown_components[cindex];
		data->session->plugin_manager->update_class(cc);
		set_string("group-component", cc.class_name + " (not found)");
		set_target("grid-variables");
		for (const auto& [i, v]: enumerate(cc.variables)) {
			add_control("Label", v.name, 0, i, "");
			add_control("Label", v.type, 1, i, "");
			add_control("Edit", v.value, 2, i, format("var-%d", i));
		}
	}
	DataWorld* data;
	int index, cindex;
};

ComponentPanel::ComponentPanel(DataWorld* _data) : Panel("component-panel") {
	from_source(R"foodelim(
Dialog solid-body-panel ''
	Grid ? '' class=card
		Expander expander 'Component' expandx
			Grid contents ''
				.
				---|
				Grid ? ''
					Label ? '' expandx
					Button edit 'Edit code' primary noexpandx
					Button delete 'Delete' danger noexpandx
)foodelim");
	data = _data;
	event("delete", [this] {
		auto e = data->entity(entity_index);
		if (entity_index >= 0 and component_index >= 0) {
			if (unknown_component)
				data->entity_remove_unknown_component(e, component_index);
			else
				data->entity_remove_component(e, component_type);
		}
	});
	event("edit", [this] {
		if (entity_index >= 0 and component_index >= 0 and user_component)
			data->session->universal_edit(FD_SCRIPT, data->entity(entity_index)->components[component_index]->component_type->owner->module->filename, false);
	});
}
void ComponentPanel::update(int _entity_index, const string& category, int _component_index) {
	entity_index = _entity_index;
	component_index = _component_index;
	unknown_component = false;
	auto e = data->entity(entity_index);
	if (category == "e")
		set_class("Entity");
	else if (category == "c")
		set_class(e->components[component_index]->component_type->name);
	else {
		unknown_component = true;
		set_class(e->get_component<EdwardTag>()->unknown_components[component_index].class_name);
	}
	set_string("expander", component_class);
}
void ComponentPanel::set_class(const string& _component_class) {
	if (_component_class == component_class)
		return;
	component_class = _component_class;
	user_component = false;
	if (content_panel) {
		unembed(content_panel);
		content_panel = nullptr;
	}

	auto e = data->entity(entity_index);
	if (unknown_component) {
		content_panel = new UnknownComponentPanel(data, entity_index, component_index);
		user_component = true;
	} else if (component_class == "Entity") {
		content_panel = new EntityBasePanel(data, entity_index);
	} else if (component_class == "ModelRef") {
		component_type = ModelRef::_class;
		content_panel = new ModelRefPanel(data, entity_index);
	} else if (component_class == "SolidBody") {
		component_type = SolidBody::_class;
		content_panel = new SolidBodyPanel(data, entity_index);
	/*} else if (component_class == "Material") {
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			content_panel = new MaterialComponentPanel(data, e.object.object->material[0], "???", [this] (const ComplexPath& p) {
			});
		} else if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
			content_panel = new MaterialComponentPanel(data, e.terrain.terrain->material.get(), e.terrain.terrain->material_file, [this, index=entity_index] (const ComplexPath& p) {
				data->entities[entity_index].terrain.save_material(data->session, p.complete);
			});
		}*/
	} else if (component_class == "MeshCollider") {
		component_type = MeshCollider::_class;
		content_panel = new DummyComponentPanel;
	} else if (component_class == "TerrainCollider") {
		component_type = TerrainRef::_class;
		content_panel = new DummyComponentPanel;
	} else if (component_class == "Skeleton") {
		component_type = Skeleton::_class;
		content_panel = new DummyComponentPanel;
	} else if (component_class == "Animator") {
		component_type = Animator::_class;
		content_panel = new DummyComponentPanel;
	} else if (component_class == "TerrainRef") {
		component_type = TerrainRef::_class;
		content_panel = new TerrainRefPanel(data, entity_index);
	} else if (component_class == "Camera") {
		component_type = Camera::_class;
		content_panel = new CameraPanel(data, entity_index);
	} else if (component_class == "Light") {
		component_type = Light::_class;
		content_panel = new LightPanel(data, entity_index);
	} else {
		content_panel = new UserComponentPanel(data, entity_index, component_index);
		user_component = true;
	}

	embed("contents", 0, 0, content_panel);
	set_visible("delete", component_class != "Entity");
	set_visible("edit", user_component);
	enable("edit", !unknown_component);
}

void ComponentPanel::set_selected(bool select) {
	expand("expander", select);
}

