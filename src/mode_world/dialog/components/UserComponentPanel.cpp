//
// Created by michi on 10/1/25.
//

#include "UserComponentPanel.h"

#include "../../data/DataWorld.h"
#include <ecs/Entity.h>
#include <stuff/PluginManager.h>
#include <lib/base/iter.h>
#include <lib/kaba/syntax/Class.h>

#include "storage/Storage.h"

const kaba::Class* find_class_member_type(const kaba::Class* c, const string& name) {
	for (const auto& e: c->elements)
		if (e.name == name)
			return e.type;
	return nullptr;
}

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
		auto t = find_class_member_type(type, v.name);
		add_control("Label", v.name, 0, i, format("l-%d", i));
		set_options(format("l-%d", i), "right,disabled");
		//add_control("Label", v.type, 1, i, "");
		const string id = format("var-%d", i);
		if (t->name == "Template*") {
			add_control("Button", "", 2, i, id);
			set_options(id, "ellipsis,expandx");
			set_string(id, str(v.value));
			event(id, [this] {
				data->session->storage->file_dialog(FD_TEMPLATE, false, true).then([this] (const ComplexPath& p) {

				});
			});
		} else if (t == kaba::common_types.f32) {
			add_control("SpinButton", "", 2, i, id);
			set_options(id, "range=::0.001");
			set_float(id, v.value.to_f32());
			event(id, [this] {
				on_edit();
			});
		} else {
			add_control("Edit", str(v.value), 2, i, id);
			event(id, [this] {
				on_edit();
			});
		}
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
		set_string(format("var-%d", i), str(v.value));
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
