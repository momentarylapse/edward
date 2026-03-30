//
// Created by michi on 9/30/25.
//

#include "RigidBodyPanel.h"
#include "../../data/DataWorld.h"
#include <y/world/components/RigidBody.h>
#include <ecs/Entity.h>
#include <stuff/PluginManager.h>



RigidBodyPanel::RigidBodyPanel(DataWorld* _data, int _index) : Node("rigid-body-panel") {
	from_source(R"foodelim(
Dialog rigid-body-panel ''
	Grid ? ''
		Label ? 'Active' right disabled
		CheckBox active '' 'tooltip=Active bodies can move and fall.\nOtherwise the body will become a static obstacle'
		---|
		Label ? 'Mass' right disabled
		SpinButton mass '' 'tooltip=Mass for gravity and inertia. Arbitrary units' range=0::0.001
		---|
		Label ? 'G-Factor' right disabled
		SpinButton g-factor '' 'tooltip=Scaling factor for effect of gravity' range=0::0.001
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
	event("g-factor", [this] { on_edit(); });
	event("theta-xx", [this] { on_edit(); });
	event("theta-xy", [this] { on_edit(); });
	event("theta-xz", [this] { on_edit(); });
	event("theta-yy", [this] { on_edit(); });
	event("theta-yz", [this] { on_edit(); });
	event("theta-zz", [this] { on_edit(); });

	data->out_changed >> create_sink([this] {
		if (!editing)
			update_ui();
	});
}
void RigidBodyPanel::update_ui() {
	auto e = data->entity(index);
	auto sb = e->get_component<RigidBody>();
	check("active", sb->dynamic);
	set_float("mass", sb->mass);
	set_float("g-factor", sb->g_factor);
	set_float("theta-xx", sb->theta_0._00);
	set_float("theta-xy", sb->theta_0._01);
	set_float("theta-xz", sb->theta_0._02);
	set_float("theta-yy", sb->theta_0._11);
	set_float("theta-yz", sb->theta_0._12);
	set_float("theta-zz", sb->theta_0._22);

	enable("mass", sb->dynamic);
	enable("g-factor", sb->dynamic);
	enable("theta-xx", sb->dynamic);
	enable("theta-xy", sb->dynamic);
	enable("theta-xz", sb->dynamic);
	enable("theta-yy", sb->dynamic);
	enable("theta-yz", sb->dynamic);
	enable("theta-zz", sb->dynamic);
}
void RigidBodyPanel::on_edit() {
	RigidBody sb;
	sb.dynamic = is_checked("active");
	sb.mass = get_float("mass");
	sb.g_factor = get_float("g-factor");
	sb.theta_0._00 = get_float("theta-xx");
	sb.theta_0._01 = get_float("theta-xy");
	sb.theta_0._02 = get_float("theta-xz");
	sb.theta_0._11 = get_float("theta-yy");
	sb.theta_0._12 = get_float("theta-yz");
	sb.theta_0._22 = get_float("theta-zz");
	sb.theta_0._10 = sb.theta_0._01;
	sb.theta_0._20 = sb.theta_0._02;
	sb.theta_0._21 = sb.theta_0._12;
	enable("mass", sb.dynamic);
	enable("g-factor", sb.dynamic);
	enable("theta-xx", sb.dynamic);
	enable("theta-xy", sb.dynamic);
	enable("theta-xz", sb.dynamic);
	enable("theta-yy", sb.dynamic);
	enable("theta-yz", sb.dynamic);
	enable("theta-zz", sb.dynamic);

	editing = true;
	auto e = data->entity(index);
	data->entity_edit_component(e, RigidBody::_class, data->session->plugin_manager->describe_class(RigidBody::_class, &sb));
	editing = false;
}
