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
		---|
		Label ? 'G-Factor' right disabled
		SpinButton g-factor '' range=0::0.001
		---|
		Label ? 'Theta' right disabled
		Grid ? ''
			SpinButton theta-xx '' range=::0.1 compact expandx
			SpinButton theta-xy '' range=::0.1 compact expandx
			SpinButton theta-xz '' range=::0.1 compact expandx
			---|
			.
			SpinButton theta-yy '' range=::0.1 compact
			SpinButton theta-yz '' range=::0.1 compact
			---|
			.
			.
			SpinButton theta-zz '' range=::0.1 compact
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
	set_float("g-factor", sb->g_factor);
	set_float("theta-xx", sb->theta_0._00);
	set_float("theta-xy", sb->theta_0._01);
	set_float("theta-xz", sb->theta_0._02);
	set_float("theta-yy", sb->theta_0._11);
	set_float("theta-yz", sb->theta_0._12);
	set_float("theta-zz", sb->theta_0._22);

	enable("mass", sb->active);
	enable("g-factor", sb->active);
	enable("theta-xx", sb->active);
	enable("theta-xy", sb->active);
	enable("theta-xz", sb->active);
	enable("theta-yy", sb->active);
	enable("theta-yz", sb->active);
	enable("theta-zz", sb->active);
}
void SolidBodyPanel::on_edit() {
	auto e = data->entity(index);
	auto sb = e->get_component<SolidBody>();
	sb->active = is_checked("active");
	sb->mass = get_float("mass");
	sb->g_factor = get_float("g-factor");
	sb->theta_0._00 = get_float("theta-xx");
	sb->theta_0._01 = get_float("theta-xy");
	sb->theta_0._02 = get_float("theta-xz");
	sb->theta_0._11 = get_float("theta-yy");
	sb->theta_0._12 = get_float("theta-yz");
	sb->theta_0._22 = get_float("theta-zz");
	sb->theta_0._10 = sb->theta_0._01;
	sb->theta_0._20 = sb->theta_0._02;
	sb->theta_0._21 = sb->theta_0._12;
	enable("mass", sb->active);
	enable("g-factor", sb->active);
	enable("theta-xx", sb->active);
	enable("theta-xy", sb->active);
	enable("theta-xz", sb->active);
	enable("theta-yy", sb->active);
	enable("theta-yz", sb->active);
	enable("theta-zz", sb->active);
}
