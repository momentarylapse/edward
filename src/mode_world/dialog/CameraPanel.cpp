//
// Created by michi on 9/30/25.
//

#include "CameraPanel.h"
#include "../data/DataWorld.h"
#include <y/world/components/Camera.h>
#include <ecs/Entity.h>

CameraPanel::CameraPanel(DataWorld* _data, int _index) : Panel("camera-panel") {
	from_source(R"foodelim(
Dialog camera-panel ''
	Grid ? ''
		Label ? 'Min distance' right disabled
		SpinButton z-min '' range=0::0.001 expandx
		---|
		Label ? 'Max distance' right disabled
		SpinButton z-max '' range=0::0.001
		---|
		Label ? 'Field of view' right disabled
		SpinButton fov '' range=0:180:0.1
		Label ? '°'
		---|
		Label ? 'Exposure' right disabled
		SpinButton z-min '' range=0:100:0.001
)foodelim");
	data = _data;
	index = _index;
	update_ui();

	event("z-min", [this] { on_edit(); });
	event("z-max", [this] { on_edit(); });
	event("fov", [this] { on_edit(); });
	event("exposure", [this] { on_edit(); });
}

void CameraPanel::update_ui() {
	auto c = data->entity(index)->get_component<Camera>();
	set_float("z-min", c->min_depth);
	set_float("z-max", c->max_depth);
	set_float("fov", c->fov * 180 / pi);
	set_float("exposure", c->exposure);
}
void CameraPanel::on_edit() {
	auto c = data->entity(index)->get_component<Camera>();
	c->min_depth = get_float("z-min");
	c->max_depth = get_float("z-max");
	c->exposure = get_float("exposure");
	c->fov = get_float("fov") * pi / 180;
}
