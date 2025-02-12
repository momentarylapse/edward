//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"
#include "AddEntityPanel.h"
#include "ModeWorld.h"
#include <helper/ResourceManager.h>
#include <y/graphics-impl.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>
#include <world/Terrain.h>

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
		for (const auto& e: mode->data->entities)
			if (e.is_selected) {
				string name = "???";
				if (e.basic_type == MultiViewType::WORLD_OBJECT)
					name = "Object " + str(e.object.filename);
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
		l.type = (LightType)get_int("type");
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
	Grid card-terrain '' class=card
		Group group-light 'Terrain'
			Grid ? ''
				Grid ? ''
					Label ? 'Filename'
					Button filename2 ''
				---|
				ListView textures 'a\\filename' nobar format=it noexpandy height=200
)foodelim");
		data = _data;
		index = _index;
		auto& e = data->entities[index];
		set_string("filename2", str(e.terrain.filename));
		reset("textures");
		for (const auto& tf: e.terrain.terrain->texture_file)
			add_string("textures", format("...\\%s", tf));

		event("textures", [this] {
			int i = get_int("textures");
			if (i >= 0)
				data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, i] (const auto& filename) {
					auto& e = data->entities[index];
					auto& t = e.terrain;
					t.terrain->texture_file[i] = filename.relative;
					t.terrain->material->textures[i] = data->session->resource_manager->load_texture(filename.relative);
					reset("textures");
					for (const auto& tf: e.terrain.terrain->texture_file)
						add_string("textures", format("...\\%s", tf));
				});
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
		Group group-terrain 'Object'
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

EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid main-grid '' expandx
)foodelim");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	add_entity_panel = new AddEntityPanel(mode_world);
	embed("main-grid", 0, 0, add_entity_panel);

	entity_list_panel = new EntityListPanel();

	mode_world->multi_view->out_selection_changed >> create_sink([this] {
		auto sel = mode_world->data->get_selection();
		cur_index = -1;

		unembed(add_entity_panel.get());
		unembed(entity_list_panel.get());
		for (auto p: component_panels)
			unembed(p);
		component_panels.clear();

		if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
			if (!add_entity_panel->owner)
				embed("main-grid", 0, 0, add_entity_panel);
		} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
			cur_index = sel[MultiViewType::WORLD_ENTITY][0];
			auto& e = mode_world->data->entities[cur_index];
			auto entity_base_panel = new EntityBasePanel(e);
			embed("main-grid", 0, 0, entity_base_panel);
			component_panels.add(entity_base_panel);

			if (e.basic_type == MultiViewType::WORLD_OBJECT) {
				auto p = new ObjectPanel(mode_world->data, cur_index);
				embed("main-grid", 0, 1, p);
				component_panels.add(p);
			} else if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
				auto terrain_panel = new TerrainPanel(mode_world->data, cur_index);
				embed("main-grid", 0, 1, terrain_panel);
				component_panels.add(terrain_panel);
			} else if (e.basic_type == MultiViewType::WORLD_CAMERA) {
				auto camera_panel = new CameraPanel(mode_world->data, cur_index);
				embed("main-grid", 0, 1, camera_panel);
				component_panels.add(camera_panel);
			} else if (e.basic_type == MultiViewType::WORLD_LIGHT) {
				auto light_panel = new LightPanel(mode_world->data, cur_index);
				embed("main-grid", 0, 1, light_panel);
				component_panels.add(light_panel);
			}
		} else {
			if (!entity_list_panel->owner)
				embed("main-grid", 0, 0, entity_list_panel);
			entity_list_panel.to<EntityListPanel>()->update(mode_world);
		}
	});

	/*mode->data->out_changed >> create_sink([this] {
	});*/
}



