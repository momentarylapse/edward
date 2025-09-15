//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"
#include "AddEntityPanel.h"
#include "../ModeWorld.h"
#include "../ModeEditTerrain.h"
#include "ComponentSelectionDialog.h"
#include <helper/ResourceManager.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>
#include <world/Model.h>
#include <world/ModelManager.h>
#include <world/Terrain.h>
#include <world/components/SolidBody.h>

#include <y/EntityManager.h>
#include <y/Entity.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>

#include <lib/kaba/syntax/Class.h>
#include <cmath>

#include "world/components/Animator.h"
#include "world/components/Collider.h"
#include "world/components/Skeleton.h"

class EntityListPanel : public xhui::Panel {
public:
	explicit EntityListPanel() : Panel("entity-list-panel") {
		from_source(R"foodelim(
Dialog entity-base-panel ''
	Grid card-entity-list '' class=card
		Group group-entity-list 'Selection'
			ListView list 'x' nobar
)foodelim");
	}
	void update(ModeWorld* mode) {
		reset("list");
		const auto& sel = mode->multi_view->selection[MultiViewType::WORLD_ENTITY];
		for (const auto& [i,e]: enumerate(mode->data->entity_manager->get_component_list<EdwardTag>()))
			if (sel.contains(i)) {
				string name = "Entity";
				if (e->owner->get_component<Camera>())
					name = "Camera";
				if (e->owner->get_component<Light>())
					name = "Light";
				if (e->owner->get_component<TerrainRef>())
					name = "Terrain";
				if (auto r = e->owner->get_component<ModelRef>()) {
					name = "Model (no file)";
					if (r->model)
						name = "Model " + str(r->filename);
				}
				add_string("list", name);
			}
	}
};

class EntityBasePanel : public xhui::Panel {
public:
	explicit EntityBasePanel(DataWorld* _data, int _index) : Panel("entity-base-panel") {
		from_source(R"foodelim(
Dialog entity-base-panel ''
	Grid ? ''
		Label ? 'Position'
		SpinButton pos-x '' range=::0.001 expandx
		---|
		.
		SpinButton pos-y '' range=::0.001
		---|
		.
		SpinButton pos-z '' range=::0.001
		---|
		Label ? 'Orientation'
		SpinButton ang-x '' range=::0.01
		Label ? '°'
		---|
		.
		SpinButton ang-y '' range=::0.01
		---|
		.
		SpinButton ang-z '' range=::0.01
)foodelim");
		data = _data;
		index = _index;
		update_ui();

		event("pos-x", [this] { on_edit(); });
		event("pos-y", [this] { on_edit(); });
		event("pos-z", [this] { on_edit(); });
		event("ang-x", [this] { on_edit(); });
		event("ang-y", [this] { on_edit(); });
		event("ang-z", [this] { on_edit(); });
	}
	void update_ui() {
		auto& e = data->entity_manager->entities[index];
		set_float("pos-x", e->pos.x);
		set_float("pos-y", e->pos.y);
		set_float("pos-z", e->pos.z);
		auto ang = e->ang.get_angles();
		set_float("ang-x", ang.x * 180 / pi);
		set_float("ang-y", ang.y * 180 / pi);
		set_float("ang-z", ang.z * 180 / pi);
	}
	void on_edit() {
		vec3 pos;
		pos.x = get_float("pos-x");
		pos.y = get_float("pos-y");
		pos.z = get_float("pos-z");
		auto ang = quaternion::rotation({get_float("ang-x") * pi / 180, get_float("ang-y") * pi / 180, get_float("ang-z") * pi / 180});
		data->edit_entity(index, pos, ang);
	}
	DataWorld* data;
	int index;
};

class CameraPanel : public xhui::Panel {
public:
	explicit CameraPanel(DataWorld* _data, int _index, int _cindex) : Panel("camera-panel") {
		from_source(R"foodelim(
Dialog camera-panel ''
	Grid ? ''
		Label ? 'Min distance'
		SpinButton z-min '' range=0::0.001 expandx
		---|
		Label ? 'Max distance'
		SpinButton z-max '' range=0::0.001
		---|
		Label ? 'Field of view'
		SpinButton fov '' range=0:180:0.1
		Label ? '°'
		---|
		Label ? 'Exposure'
		SpinButton z-min '' range=0:100:0.001
)foodelim");
		data = _data;
		index = _index;
		cindex = _cindex;
		update_ui();

		event("z-min", [this] { on_edit(); });
		event("z-max", [this] { on_edit(); });
		event("fov", [this] { on_edit(); });
		event("exposure", [this] { on_edit(); });
	}
	DataWorld* data;
	int index, cindex;;

	void update_ui() {
		auto c = data->entity_manager->entities[index]->get_component<Camera>();
		set_float("z-min", c->min_depth);
		set_float("z-max", c->max_depth);
		set_float("fov", c->fov * 180 / pi);
		set_float("exposure", c->exposure);
	}
	void on_edit() {
		/*auto& c = data->entities[index].get("Camera");
		c.set("min_depth", "f32", f2s(get_float("z-min"), 3));
		c.set("max_depth", "f32", f2s(get_float("z-max"), 3));
		c.set("exposure", "f32", f2s(get_float("exposure"), 3));
		c.set("fov", "f32", f2s(get_float("fov") * pi / 180, 3));
		data->entity_edit_component(index, cindex, c);*/
	}
};

class LightPanel : public obs::Node<xhui::Panel> {
public:
	explicit LightPanel(DataWorld* _data, int _index) : obs::Node<xhui::Panel>("light-panel") {
		from_source(R"foodelim(
Dialog light-panel ''
	Grid ? ''
		Label ? 'Type'
		ComboBox type 'Directional\\Point\\Cone' range=0:2:1
		---|
		Label ? 'Radius'
		SpinButton radius '' range=0::0.1
		---|
		Label ? 'Theta'
		SpinButton theta '' range=0:180:0.1
		Label ? '°'
		---|
		Label ? 'Color'
		ColorButton color ''
		---|
		Label ? 'Power'
		SpinButton power '' range=0::0.1
		---|
		Label ? 'Harshness'
		SpinButton harshness '' range=0:100:1
		Label ? '%'
)foodelim");
		data = _data;
		index = _index;
		update_ui();
		/*data->out_changed >> create_sink([this] {
			msg_write("update");
			update_ui();
		});*/

		event("type", [this] { on_edit(); });
		event("radius", [this] { on_edit(); });
		event("theta", [this] { on_edit(); });
		event("color", [this] { on_edit(); });
		event("power", [this] { on_edit(); });
		event("harshness", [this] { on_edit(); });
	}
	DataWorld* data;
	int index;

	void update_ui() {
		auto e = data->entity_manager->entities[index];
		auto l = e->get_component<Light>();
		float b = l->light.light.col.brightness();
		set_int("type", (int)l->light.type);
		set_float("radius", sqrtf(b*b) / 10);
		set_float("harshness", l->light.light.harshness * 100);
		set_float("theta", max(l->light.light.theta * 180 / pi, 0.0f));
		set_color("color", l->light.light.col * (1.0f / b));
		set_float("power", b);
		enable("power", l->light.type == yrenderer::LightType::DIRECTIONAL);
		enable("radius", l->light.type != yrenderer::LightType::DIRECTIONAL);
		enable("theta", l->light.type == yrenderer::LightType::CONE);
	}

	void on_edit() {
		auto e = data->entity_manager->entities[index];
		auto l = e->get_component<Light>();
		l->light.type = (yrenderer::LightType)get_int("type");
		enable("power", l->light.type == yrenderer::LightType::DIRECTIONAL);
		enable("radius", l->light.type != yrenderer::LightType::DIRECTIONAL);
		enable("theta", l->light.type == yrenderer::LightType::CONE);

		float radius = get_float("radius");
		float power = get_float("power");
		if (l->light.type == yrenderer::LightType::DIRECTIONAL) {
			l->light.light.col = get_color("color") * power;
			l->light.light.radius = -1;
			l->light.light.theta = -1;
		} else {
			l->light.light.col = get_color("color") * (radius * radius / 100);
			l->light.light.radius = radius;
			l->light.light.theta = get_float("theta") * pi / 180;
		}
		l->light.light.harshness = get_float("harshness") / 100;
		//data->edit_light(index, l);*/
	}
};

class TerrainPanel : public xhui::Panel {
public:
	explicit TerrainPanel(DataWorld* _data, int _index) : Panel("terrain-panel") {
		from_source(R"foodelim(
Dialog terrain-panel ''
	Grid ? ''
		Grid ? ''
			Label ? 'Filename'
			Button filename ''
			---|
			Label ? 'Size X'
			SpinButton size-x '' range=::0.1
			---|
			Label ? 'Size Z'
			SpinButton size-z '' range=::0.1
			---|
			Label ? 'Cells'
			Label cells ''
		---|
		Grid ? ''
			Button edit 'edit...'
)foodelim");
		data = _data;
		index = _index;
		auto e = data->entity(index);
		auto tr = e->get_component<TerrainRef>();
		set_string("filename", str(tr->filename));
		if (auto t = tr->terrain) {
			set_float("size-x", t->pattern.x * (float)t->num_x);
			set_float("size-z", t->pattern.z * (float)t->num_z);
			set_string("cells", format("%d x %d", t->num_x, t->num_z));
		}
		event("size-x", [this] {
			auto e = data->entity(index);
			if (auto t = e->get_component<TerrainRef>()->terrain)
				data->edit_terrain_meta_data(index, {get_float("size-x") / (float)t->num_x, 0, t->pattern.z});
		});
		event("size-z", [this] {
			auto e = data->entity(index);
			if (auto t = e->get_component<TerrainRef>()->terrain)
				data->edit_terrain_meta_data(index, {t->pattern.x, 0, get_float("size-z") / (float)t->num_z});
		});
		event("edit", [this] {
			data->session->set_mode(new ModeEditTerrain(data->session->mode_world, index));
		});
	}
	DataWorld* data;
	int index;
};

class ModelPanel : public xhui::Panel {
public:
	explicit ModelPanel(DataWorld* _data, int _index) : Panel("model-panel") {
		from_source(R"foodelim(
Dialog model-panel ''
	Grid ? ''
		Label ? 'Filename'
		Button filename '' expandx
)foodelim");
		data = _data;
		index = _index;
		auto e = data->entity_manager->entities[index];
		auto m = e->get_component<ModelRef>();
		set_string("filename", str(m->filename));
	}
	DataWorld* data;
	int index;
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

void update_class(Session* session, ScriptInstanceData& _c);

class UserComponentPanel : public xhui::Panel {
public:
	explicit UserComponentPanel(DataWorld* _data, int _index, int _cindex) : Panel("user-component-panel") {
		from_source(R"foodelim(
Dialog user-component-panel ''
	Grid grid-variables ''
)foodelim");
		data = _data;
		index = _index;
		cindex = _cindex;

		auto e = data->entity(index);
		auto& cc = e->get_component<EdwardTag>()->user_components[cindex];
		update_class(data->session, cc);
		set_string("group-component", cc.class_name);
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

class SolidBodyPanel : public xhui::Panel {
public:
	explicit SolidBodyPanel(DataWorld* _data, int _index) : Panel("solid-body-panel") {
		from_source(R"foodelim(
Dialog solid-body-panel ''
	Grid ? ''
		Label ? 'Active'
		CheckBox active ''
		---|
		Label ? 'Mass'
		SpinButton mass '' range=0::0.001
)foodelim");
		data = _data;
		index = _index;
		auto e = data->entity(index);
		auto sb = e->get_component<SolidBody>();
		check("active", sb->active);
		set_float("mass", sb->mass);
		enable("mass", sb->active);
	}
	DataWorld* data;
	int index;
};

class DummyComponentPanel : public xhui::Panel {
	public:
	DummyComponentPanel() : Panel("dummy-panel") {
		add_control("Label", "!italic,expandx,center\\no configuration", 0, 0, "message");
	}
};

class ComponentPanel : public xhui::Panel {
public:
	explicit ComponentPanel(DataWorld* _data) : Panel("component-panel") {
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
			if (entity_index >= 0 and component_index >= 0) {
				if (user_component)
					data->entity_remove_user_component(entity_index, component_index);
				else
					data->entity_remove_component(entity_index, component_type);
			}
		});
		event("edit", [this] {
			if (entity_index >= 0 and component_index >= 0 and user_component)
				data->session->edit_code_file(data->entity(entity_index)->get_component<EdwardTag>()->user_components[component_index].filename);
		});
	}
	void update(int _entity_index, const string& category, int _component_index) {
		entity_index = _entity_index;
		component_index = _component_index;
		auto e = data->entity_manager->entities[entity_index];
		if (category == "e")
			set_class("Entity");
		else if (category == "c")
			set_class(e->components[component_index]->component_type->name);
		else
			set_class(e->get_component<EdwardTag>()->user_components[component_index].class_name);
		set_string("expander", component_class);
	}
	void set_class(const string& _component_class) {
		if (_component_class == component_class)
			return;
		component_class = _component_class;
		user_component = false;
		if (content_panel) {
			unembed(content_panel);
			content_panel = nullptr;
		}

		auto e = data->entity_manager->entities[entity_index];
		if (component_class == "Entity") {
			content_panel = new EntityBasePanel(data, entity_index);
		} else if (component_class == "ModelRef") {
			component_type = ModelRef::_class;
			content_panel = new ModelPanel(data, entity_index);
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
			content_panel = new TerrainPanel(data, entity_index);
		} else if (component_class == "Camera") {
			component_type = Camera::_class;
			content_panel = new CameraPanel(data, entity_index, component_index);
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
	}
	void set_selected(bool select) {
		expand("expander", select);
	}
	DataWorld* data;
	int entity_index = -1;
	int component_index = -1;
	const kaba::Class* component_type;
	string component_class;
	Panel* content_panel = nullptr;
	bool user_component = false;
};



EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid main-grid '' expandx
		.
		---|
		ListView components 'c' nobar sunkenbackground=no showselection=no selectsingle hidden
		---|
		Button add-component '+' hidden
)foodelim");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	add_entity_panel = new AddEntityPanel(mode_world);
	embed("main-grid", 0, 0, add_entity_panel);

	entity_list_panel = new EntityListPanel();

	auto component_list = (xhui::ListView*)get_control("components");
	component_list->column_factories[0].f_create = [this](const string& id) -> xhui::Control* {
		return new ComponentPanel(mode_world->data);
	};
	component_list->column_factories[0].f_set = [this](xhui::Control* c, const string& t) {
		const auto xx = t.explode(":");
		reinterpret_cast<ComponentPanel*>(c)->update(xx[0]._int(), xx[1], xx[2]._int());
	};
	component_list->column_factories[0].f_select = [this](xhui::Control* c, bool selected) {
		reinterpret_cast<ComponentPanel*>(c)->set_selected(selected);
	};

	mode_world->data->out_component_added >> create_sink([this] {
		update(true);
	});
	mode_world->data->out_component_removed >> create_sink([this] {
		update(true);
	});
	mode_world->multi_view->out_selection_changed >> create_sink([this] {
		update(false);
	});

	event("add-component", [this] {
		ComponentSelectionDialog::ask(this, mode_world->session).then([this] (const ScriptInstanceData& c) {
			if (c.filename.is_in("y"))
				mode_world->data->entity_add_component_generic(cur_index, mode_world->data->entity_manager->component_manager->f_parse_type(c.class_name));
			else
				mode_world->data->entity_add_user_component(cur_index, c);
		});
	});

	/*mode->data->out_changed >> create_sink([this] {
	});*/
}

void EntityPanel::update(bool force) {
	const auto& sel = mode_world->multi_view->selection;

	unembed(add_entity_panel.get());
	unembed(entity_list_panel.get());

	if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
		cur_index = -1;
		reset("components");
		set_visible("components", false);
		set_visible("add-component", false);
		if (!add_entity_panel->owner)
			embed("main-grid", 0, 0, add_entity_panel);
	} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
		int next = sel[MultiViewType::WORLD_ENTITY][0];
		if (next == cur_index and !force)
			return;
		cur_index = next;
		reset("components");
		auto e = mode_world->data->entity_manager->entities[cur_index];
		set_options("components-viewport", "expandy");
		set_visible("components", true);
		set_visible("add-component", true);

		add_string("components", format("%d:e:0", cur_index)); // Entity...
		for (int j=0; j<e->components.num; j++)
			if (e->components[j]->component_type != EdwardTag::_class)
				add_string("components", format("%d:c:%d", cur_index, j));
		for (int j=0; j<e->get_component<EdwardTag>()->user_components.num; j++)
			add_string("components", format("%d:u:%d", cur_index, j));

		set_int("components", 0);
	} else {
		cur_index = -1;
		reset("components");
		set_visible("components", false);
		set_visible("add-component", false);
		if (!entity_list_panel->owner)
			embed("main-grid", 0, 0, entity_list_panel);
		entity_list_panel.to<EntityListPanel>()->update(mode_world);
	}
}




