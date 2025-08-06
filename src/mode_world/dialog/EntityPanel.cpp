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
		for (const auto& [i,e]: enumerate(mode->data->entities))
			if (sel.contains(i)) {
				string name = "???";
				if (e.basic_type == MultiViewType::WORLD_OBJECT)
					name = "Model " + str(e.object.filename);
				else if (e.basic_type == MultiViewType::WORLD_TERRAIN)
					name = "Terrain " + str(e.terrain.filename);
				else if (e.basic_type == MultiViewType::WORLD_LIGHT)
					name = "Light";
				else if (e.basic_type == MultiViewType::WORLD_CAMERA)
					name = "Camera";
				else if (e.basic_type == MultiViewType::WORLD_ENTITY)
					name = "Entity";
				add_string("list", name);
			}
	}
};

class EntityBasePanel : public xhui::Panel {
public:
	explicit EntityBasePanel(WorldEntity& e) : Panel("entity-base-panel") {
		from_source(R"foodelim(
Dialog entity-base-panel ''
	Grid card-entity '' class=card
		Group group-entity 'Entity'
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
				SpinButton ang-x '' range=::0.001
				---|
				.
				SpinButton ang-y '' range=::0.001
				---|
				.
				SpinButton ang-z '' range=::0.001
)foodelim");
		set_float("pos-x", e.pos.x);
		set_float("pos-y", e.pos.y);
		set_float("pos-z", e.pos.z);
		auto ang = e.ang.get_angles();
		set_float("ang-x", ang.x * 180 / pi);
		set_float("ang-y", ang.y * 180 / pi);
		set_float("ang-z", ang.z * 180 / pi);
	}
};

class CameraPanel : public xhui::Panel {
public:
	explicit CameraPanel(DataWorld* _data, int _index) : Panel("camera-panel") {
		from_source(R"foodelim(
Dialog camera-panel ''
	Grid card-camera '' class=card
		Group group-camera 'Camera'
			Grid ? ''
				Label ? 'Min distance'
				SpinButton z-min '' range=0::0.001
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
		auto& e = data->entities[index];
		set_float("z-min", e.camera.min_depth);
		set_float("z-max", e.camera.max_depth);
		set_float("fov", e.camera.fov * 180 / pi);
		set_float("exposure", e.camera.exposure);

		event("z-min", [this] { on_edit(); });
		event("z-max", [this] { on_edit(); });
		event("fov", [this] { on_edit(); });
		event("exposure", [this] { on_edit(); });
	}
	DataWorld* data;
	int index;

	void on_edit() {
		auto& e = data->entities[index];
		auto c = e.camera;
		c.min_depth = get_float("z-min");
		c.max_depth = get_float("z-max");
		c.exposure = get_float("exposure");
		c.fov = get_float("fov") * pi / 180;
		data->edit_camera(index, c);
	}
};

class LightPanel : public xhui::Panel {
public:
	explicit LightPanel(DataWorld* _data, int _index) : Panel("light-panel") {
		from_source(R"foodelim(
Dialog light-panel ''
	Grid card-light '' class=card
		Group group-light 'Light'
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
		auto& e = data->entities[index];
		set_int("type", (int)e.light.type);
		set_float("radius", e.light.radius);
		set_float("harshness", e.light.harshness * 100);
		set_float("theta", e.light.theta * 180 / pi);
		set_float("radius", e.light.radius);
		set_color("color", e.light.col);
		set_float("power", e.light.col.r + e.light.col.g + e.light.col.b);

		event("type", [this] { on_edit(); });
		event("radius", [this] { on_edit(); });
		event("theta", [this] { on_edit(); });
		event("color", [this] { on_edit(); });
		event("power", [this] { on_edit(); });
		event("harshness", [this] { on_edit(); });
	}
	DataWorld* data;
	int index;

	void on_edit() {
		auto& e = data->entities[index];
		auto c = e.camera;
		auto l = e.light;
		l.type = (yrenderer::LightType)get_int("type");
		l.radius = get_float("radius");
		l.theta = get_float("theta") * pi / 180;
		l.harshness = get_float("harshness") / 100;
		l.col = get_color("color");
		data->edit_light(index, l);
	}
};

class TerrainPanel : public xhui::Panel {
public:
	explicit TerrainPanel(DataWorld* _data, int _index) : Panel("terrain-panel") {
		from_source(R"foodelim(
Dialog terrain-panel ''
	Grid ? '' class=card
		Group ? 'Terrain'
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
		auto& e = data->entities[index];
		auto& t = e.terrain;
		set_string("filename", str(t.filename));
		set_float("size-x", t.terrain->pattern.x * (float)t.terrain->num_x);
		set_float("size-z", t.terrain->pattern.z * (float)t.terrain->num_z);
		set_string("cells", format("%d x %d", t.terrain->num_x, t.terrain->num_z));
		event("size-x", [this] {
			auto& e = data->entities[index];
			auto& t = e.terrain;
			data->edit_terrain_meta_data(index, {get_float("size-x") / (float)t.terrain->num_x, 0, t.terrain->pattern.z});
		});
		event("size-z", [this] {
			auto& e = data->entities[index];
			auto& t = e.terrain;
			data->edit_terrain_meta_data(index, {t.terrain->pattern.x, 0, get_float("size-z") / (float)t.terrain->num_z});
		});
		event("edit", [this] {
			data->session->set_mode(new ModeEditTerrain(data->session->mode_world, index));
		});
	}
	DataWorld* data;
	int index;
};

class ObjectPanel : public xhui::Panel {
public:
	explicit ObjectPanel(DataWorld* _data, int _index) : Panel("object-panel") {
		from_source(R"foodelim(
Dialog object-panel ''
	Grid card-object '' class=card
		Group group-terrain 'Model'
			Grid ? ''
				Label ? 'Filename'
				Button filename '' expandx
)foodelim");
		data = _data;
		index = _index;
		auto& e = data->entities[index];
		set_string("filename", str(e.object.filename));
	}
	DataWorld* data;
	int index;
};

class MaterialComponentPanel : public xhui::Panel {
public:
	explicit MaterialComponentPanel(DataWorld* _data, yrenderer::Material* m, const Path& filename, std::function<void(const ComplexPath&)> _f_save) : Panel("material-panel") {
		from_source(R"foodelim(
Dialog material-panel ''
	Grid card-terrain-material '' class=card
		Group group-material 'Material'
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
	Grid card-component '' class=card
		Group group-component '...'
			Grid grid-variables ''
)foodelim");
		data = _data;
		index = _index;
		cindex = _cindex;

		auto& e = data->entities[index];
		auto& cc = e.components[cindex];
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
	explicit SolidBodyPanel(DataWorld* _data, int _index, bool _in_model) : Panel("solid-body-panel") {
		from_source(R"foodelim(
Dialog solid-body-panel ''
	Grid ? '' class=card
		Group ? 'SolidBody'
			Grid ? ''
				Label ? 'Mass'
				SpinButton mass '' range=0::0.001
				---|
				Label ? 'Active'
				CheckBox active ''
)foodelim");
		data = _data;
		index = _index;
		auto& e = data->entities[index];
		if (_in_model) {
			set_float("mass", e.object.object->_template->solid_body->mass);
			check("active", e.object.object->_template->solid_body->active);
		}
	}
	DataWorld* data;
	int index;
};

class MeshColliderPanel : public xhui::Panel {
public:
	explicit MeshColliderPanel(DataWorld* _data, int _index, bool _in_model) : Panel("collider-panel") {
		from_source(R"foodelim(
Dialog collider-panel ''
	Grid ? '' class=card
		Group ? 'MeshCollider'
			Grid ? ''
)foodelim");
		data = _data;
		index = _index;
		auto& e = data->entities[index];
		if (_in_model) {
		}
	}
	DataWorld* data;
	int index;
};

class AnimatorPanel : public xhui::Panel {
public:
	explicit AnimatorPanel(DataWorld* _data, int _index, bool _in_model) : Panel("collider-panel") {
		from_source(R"foodelim(
Dialog collider-panel ''
	Grid ? '' class=card
		Group ? 'Animator'
			Grid ? ''
)foodelim");
		data = _data;
		index = _index;
		auto& e = data->entities[index];
		if (_in_model) {
		}
	}
	DataWorld* data;
	int index;
};

class SkeletonPanel : public xhui::Panel {
public:
	explicit SkeletonPanel(DataWorld* _data, int _index, bool _in_model) : Panel("collider-panel") {
		from_source(R"foodelim(
Dialog collider-panel ''
	Grid ? '' class=card
		Group ? 'Skeleton'
			Grid ? ''
)foodelim");
		data = _data;
		index = _index;
		auto& e = data->entities[index];
		if (_in_model) {
		}
	}
	DataWorld* data;
	int index;
};

EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid main-grid '' expandx
		.
		---|
		Viewport components-viewport '' noexpandy hidden
			Grid components-grid ''
)foodelim");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	add_entity_panel = new AddEntityPanel(mode_world);
	embed("main-grid", 0, 0, add_entity_panel);

	entity_list_panel = new EntityListPanel();

	mode_world->multi_view->out_selection_changed >> create_sink([this] {
		const auto& sel = mode_world->multi_view->selection;
		cur_index = -1;

		unembed(add_entity_panel.get());
		unembed(entity_list_panel.get());
		for (auto p: component_panels)
			unembed(p);
		component_panels.clear();
		remove_control("add-component");
		set_options("components-viewport", "noexpandy");

		auto add_component_panel = [this] (Panel* p) {
			embed("components-grid", 0, component_panels.num + 1, p);
			component_panels.add(p);
		};

		if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
			if (!add_entity_panel->owner)
				embed("main-grid", 0, 0, add_entity_panel);
		} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
			cur_index = sel[MultiViewType::WORLD_ENTITY][0];
			auto& e = mode_world->data->entities[cur_index];
			set_options("components-viewport", "expandy");
			add_component_panel(new EntityBasePanel(e));

			if (e.basic_type == MultiViewType::WORLD_OBJECT) {
				add_component_panel(new ObjectPanel(mode_world->data, cur_index));
				add_component_panel(new MaterialComponentPanel(mode_world->data, e.object.object->material[0], "???", [this] (const ComplexPath& p) {
				}));
				if (e.object.object->_template->solid_body)
					add_component_panel(new SolidBodyPanel(mode_world->data, cur_index, true));
				if (e.object.object->_template->mesh_collider)
					add_component_panel(new MeshColliderPanel(mode_world->data, cur_index, true));
				if (e.object.object->_template->skeleton)
					add_component_panel(new SkeletonPanel(mode_world->data, cur_index, true));
				if (e.object.object->_template->animator)
					add_component_panel(new AnimatorPanel(mode_world->data, cur_index, true));
			} else if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
				add_component_panel(new TerrainPanel(mode_world->data, cur_index));
				add_component_panel(new MaterialComponentPanel(mode_world->data, e.terrain.terrain->material.get(), e.terrain.terrain->material_file, [this, &e] (const ComplexPath& p) {
					e.terrain.save_material(mode_world->session, p.complete);
				}));
				add_component_panel(new SolidBodyPanel(mode_world->data, cur_index, false)); // FIXME
				add_component_panel(new MeshColliderPanel(mode_world->data, cur_index, true));
			} else if (e.basic_type == MultiViewType::WORLD_CAMERA) {
				add_component_panel(new CameraPanel(mode_world->data, cur_index));
			} else if (e.basic_type == MultiViewType::WORLD_LIGHT) {
				add_component_panel(new LightPanel(mode_world->data, cur_index));
			}
			for (int i=0; i<e.components.num; i++)
				add_component_panel(new UserComponentPanel(mode_world->data, cur_index, i));
			add_control("Button", "+", 0, 1 + component_panels.num, "add-component");
		} else {
			if (!entity_list_panel->owner)
				embed("main-grid", 0, 0, entity_list_panel);
			entity_list_panel.to<EntityListPanel>()->update(mode_world);
		}
	});

	event("add-component", [this] {
		ComponentSelectionDialog::ask(this, mode_world->session).then([this] (const ScriptInstanceData& c) {
			mode_world->data->entity_add_component(cur_index, c);
		});
	});

	/*mode->data->out_changed >> create_sink([this] {
	});*/
}



