//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"

#include <lib/os/msg.h>
#include <view/MultiView.h>

#include "ModeWorld.h"

EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid ? '' expandx
		Grid card-entity '' class=card visible=no
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
		---|
		Grid card-object '' class=card visible=no
			Group group-terrain 'Object'
				Grid ? ''
					Label ? 'Filename'
					Button filename '' expandx
		---|
		Grid card-terrain '' class=card visible=no
			Group group-terrain 'Terrain'
				Grid ? ''
					Label ? 'Filename'
					Button filename2 ''
		---|
		Grid card-camera '' class=card visible=no
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
		---|
		Grid card-light '' class=card visible=no
			Group group-light 'Light'
				Grid ? ''
					Label ? 'Type'
					SpinButton type '' range=0:2:1
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
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	mode_world->multi_view->out_selection_changed >> create_sink([this] {
		auto sel = mode_world->data->get_selection();
		cur_index = -1;
		bool ok = (sel[MultiViewType::WORLD_ENTITY].num == 1);

		if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
			cur_index = sel[MultiViewType::WORLD_ENTITY][0];
			auto& e = mode_world->data->entities[cur_index];
			set_float("pos-x", e.pos.x);
			set_float("pos-y", e.pos.y);
			set_float("pos-z", e.pos.z);
			auto ang = e.ang.get_angles();
			set_float("ang-x", e.ang.x * 180 / pi);
			set_float("ang-y", e.ang.y * 180 / pi);
			set_float("ang-z", e.ang.z * 180 / pi);
			set_visible("card-entity", true);
			set_visible("card-object", e.basic_type == MultiViewType::WORLD_OBJECT);
			set_visible("card-terrain", e.basic_type == MultiViewType::WORLD_TERRAIN);
			set_visible("card-camera", e.basic_type == MultiViewType::WORLD_CAMERA);
			set_visible("card-light", e.basic_type == MultiViewType::WORLD_LIGHT);
			if (e.basic_type == MultiViewType::WORLD_OBJECT) {
				set_string("filename", str(e.object.filename));
			} else if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
				set_string("filename2", str(e.terrain.filename));
			} else if (e.basic_type == MultiViewType::WORLD_CAMERA) {
				set_float("z-min", e.camera.min_depth);
				set_float("z-max", e.camera.max_depth);
				set_float("fov", e.camera.fov * 180 / pi);
				set_float("exposure", e.camera.exposure);
			} else if (e.basic_type == MultiViewType::WORLD_LIGHT) {
				set_int("type", (int)e.light.type);
				set_float("radius", e.light.radius);
				set_float("harshness", e.light.harshness * 100);
				set_float("theta", e.light.theta * 180 / pi);
				set_float("radius", e.light.radius);
				set_color("color", e.light.col);
				set_float("power", e.light.col.r + e.light.col.g + e.light.col.b);
			}
		} else {
			set_visible("card-entity", false);
			set_visible("card-object", false);
			set_visible("card-terrain", false);
			set_visible("card-camera", false);
			set_visible("card-light", false);
		}
	});
	event("type", [this] { on_edit_light(); });
	event("radius", [this] { on_edit_light(); });
	event("theta", [this] { on_edit_light(); });
	event("color", [this] { on_edit_light(); });
	event("power", [this] { on_edit_light(); });
	event("harshness", [this] { on_edit_light(); });

	event("z-min", [this] { on_edit_camera(); });
	event("z-max", [this] { on_edit_camera(); });
	event("fov", [this] { on_edit_camera(); });
	event("exposure", [this] { on_edit_camera(); });

	/*mode->data->out_changed >> create_sink([this] {
	});*/
}

void EntityPanel::on_edit_light() {
	if (cur_index < 0)
		return;
	auto& e = mode_world->data->entities[cur_index];
	auto l = e.light;
	l.type = (LightType)get_int("type");
	l.radius = get_float("radius");
	l.theta = get_float("theta") * pi / 180;
	l.harshness = get_float("harshness") / 100;
	l.col = get_color("color");
	mode_world->data->edit_light(cur_index, l);
}

void EntityPanel::on_edit_camera() {
	if (cur_index < 0)
		return;
	auto& e = mode_world->data->entities[cur_index];
	auto c = e.camera;
	c.min_depth = get_float("z-min");
	c.max_depth = get_float("z-max");
	c.exposure = get_float("exposure");
	c.fov = get_float("fov") * pi / 180;
	mode_world->data->edit_camera(cur_index, c);
}



