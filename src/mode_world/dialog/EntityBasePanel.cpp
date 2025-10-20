//
// Created by michi on 10/1/25.
//

#include "EntityBasePanel.h"
#include "../data/DataWorld.h"
#include <ecs/Entity.h>


EntityBasePanel::EntityBasePanel(DataWorld* _data, int _index) : Panel("entity-base-panel") {
	from_source(R"foodelim(
Dialog entity-base-panel ''
	Grid ? ''
		Label ? 'Position' right disabled
		SpinButton pos-x '' range=::0.001 expandx
		---|
		.
		SpinButton pos-y '' range=::0.001
		---|
		.
		SpinButton pos-z '' range=::0.001
		---|
		Label ? 'Orientation' right disabled
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

void EntityBasePanel::update_ui() {
	auto e = data->entity(index);
	set_float("pos-x", e->pos.x);
	set_float("pos-y", e->pos.y);
	set_float("pos-z", e->pos.z);
	auto ang = e->ang.get_angles();
	set_float("ang-x", ang.x * 180 / pi);
	set_float("ang-y", ang.y * 180 / pi);
	set_float("ang-z", ang.z * 180 / pi);
}

void EntityBasePanel::on_edit() {
	auto e = data->entity(index);
	vec3 pos;
	pos.x = get_float("pos-x");
	pos.y = get_float("pos-y");
	pos.z = get_float("pos-z");
	auto ang = quaternion::rotation({get_float("ang-x") * pi / 180, get_float("ang-y") * pi / 180, get_float("ang-z") * pi / 180});
	data->edit_entity(e, pos, ang);
}

