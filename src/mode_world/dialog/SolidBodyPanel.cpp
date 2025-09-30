//
// Created by michi on 9/30/25.
//

#include "SolidBodyPanel.h"
#include "../data/DataWorld.h"
#include <y/world/components/SolidBody.h>
#include <y/y/Entity.h>


SolidBodyPanel::SolidBodyPanel(DataWorld* _data, int _index) : Panel("solid-body-panel") {
	from_source(R"foodelim(
Dialog solid-body-panel ''
	Grid ? ''
		Label ? 'Active' right disabled
		CheckBox active ''
		---|
		Label ? 'Mass' right disabled
		SpinButton mass '' range=0::0.001
)foodelim");
	data = _data;
	index = _index;
	update_ui();
	event("active", [this] { on_edit(); });
	event("mass", [this] { on_edit(); });
}
void SolidBodyPanel::update_ui() {
	auto e = data->entity(index);
	auto sb = e->get_component<SolidBody>();
	check("active", sb->active);
	set_float("mass", sb->mass);
	enable("mass", sb->active);
}
void SolidBodyPanel::on_edit() {
	auto e = data->entity(index);
	auto sb = e->get_component<SolidBody>();
	sb->active = is_checked("active");
	sb->mass = get_float("mass");
	enable("mass", sb->active);
}
