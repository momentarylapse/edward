//
// Created by michi on 3/30/26.
//

#include "LinkPanel.h"
#include "../../data/DataWorld.h"
#include <y/world/components/Link.h>
#include <stuff/PluginManager.h>
#include <ecs/Entity.h>

LinkPanel::LinkPanel(DataWorld* _data, int _index) : Node("link-panel") {
	from_source(R"foodelim(
Dialog link-panel ''
	Grid ? ''
		Label ? 'Type' right disabled
		ComboBox type 'None\\Socket\\Hinge\\Universal\\Spring\\Slider' 'tooltip=Type of physical link/joint' expandx
		---|
		Label ? 'Entity 1' right disabled
		SpinButton a '' 'tooltip=Index of first attached entity (needs a RigidBody component)' range=0::
		---|
		Label ? 'Entity 2' right disabled
		SpinButton b '' 'tooltip=Index of second attached entity (needs a RigidBody component)' range=0::
)foodelim");
	data = _data;
	index = _index;
	update_ui();

	event("type", [this] { on_edit(); });
	event("a", [this] { on_edit(); });
	event("b", [this] { on_edit(); });

	data->out_changed >> create_sink([this] {
		if (!editing)
			update_ui();
	});
}

void LinkPanel::update_ui() {
	auto e = data->entity(index);
	auto l = e->get_component<Link>();
	set_int("type", (int)l->link_type);
	set_int("a", l->a);
	set_int("b", l->b);
}

void LinkPanel::on_edit() {
	Link l;
	l.link_type = (LinkType)get_int("type");
	l.a = get_int("a");
	l.b = get_int("b");

	editing = true;
	auto e = data->entity(index);
	data->entity_edit_component(e, Link::_class, data->session->plugin_manager->describe_class(Link::_class, &l));
	editing = false;
}

