//
// Created by michi on 10/1/25.
//

#include "UserComponentPanel.h"

#include "../data/DataWorld.h"
#include <ecs/Entity.h>
#include <stuff/PluginManager.h>
#include <lib/base/iter.h>
#include <lib/kaba/syntax/Class.h>

UserComponentPanel::UserComponentPanel(DataWorld* _data, int _index, int _cindex) : Node("user-component-panel") {
	from_source(R"foodelim(
Dialog user-component-panel ''
	Grid grid-variables ''
)foodelim");
	data = _data;
	index = _index;
	cindex = _cindex;

	auto e = data->entity(index);
	auto c = e->components[cindex];
	auto type = c->component_type;
	set_string("group-component", type->name);
	set_target("grid-variables");
	const auto desc = data->session->plugin_manager->describe_class(type, c);
	for (const auto& [i, v]: enumerate(desc.variables)) {
		add_control("Label", v.name, 0, i, format("l-%d", i));
		set_options(format("l-%d", i), "right,disabled");
		add_control("Label", v.type, 1, i, "");
		add_control("Edit", v.value, 2, i, format("var-%d", i));

		event(format("var-%d", i), [this] {
			on_edit();
		});
	}

	update_ui();

	data->out_changed >> create_sink([this] {
		if (!editing)
			update_ui();
	});
}

void UserComponentPanel::update_ui() {
	auto e = data->entity(index);
	auto c = e->components[cindex];
	auto type = c->component_type;
	const auto desc = data->session->plugin_manager->describe_class(type, c);
	for (const auto& [i, v]: enumerate(desc.variables)) {
		set_string(format("var-%d", i), v.value);
	}
}

void UserComponentPanel::on_edit() {
	auto e = data->entity(index);
	auto c = e->components[cindex];
	auto type = c->component_type;
	auto desc = data->session->plugin_manager->describe_class(type, c);
	for (auto&& [i, v]: enumerate(desc.variables)) {
		v.value = get_string(format("var-%d", i));
	}

	editing = true;
	data->entity_edit_component(e, type, desc);
	editing = false;
}
