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

UserComponentPanel::UserComponentPanel(DataWorld* _data, int _index, const kaba::Class* _type) : ComponentContentsPanel(_data, _index) {
	from_source(R"foodelim(
Dialog user-component-panel ''
	Grid grid-variables ''
)foodelim");
	type = _type;

	auto e = data->entity(index);
	auto c = e->_get_component_generic_(type);
	if (!c)
		return;
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
			event(id, [this] {
				data->session->storage->file_dialog(FD_TEMPLATE, false, true).then([this] (const ComplexPath& p) {

				});
			});
			continue;
		} else if (t == kaba::common_types.f32) {
			add_control("SpinButton", "", 2, i, id);
			set_options(id, "range=::0.001");
		} else if (t == kaba::common_types.i32) {
			add_control("SpinButton", "", 2, i, id);
		} else if (t == kaba::common_types._bool) {
			add_control("CheckBox", "", 2, i, id);
		} else {
			add_control("Edit", "", 2, i, id);
		}
		event(id, [this] {
			on_edit();
		});
	}
}

void UserComponentPanel::update_ui() {
	auto e = data->entity(index);
	if (auto c = e->_get_component_generic_(type)) {
		const auto desc = data->session->plugin_manager->describe_class(type, c);
		for (const auto& [i, v]: enumerate(desc.variables)) {
			const string id = format("var-%d", i);
			auto t = find_class_member_type(type, v.name);
			if (t->name == "Template*") {
				set_string(id, str(v.value));
			} else if (t == kaba::common_types.f32) {
				set_float(id, v.value.to_f32());
			} else if (t == kaba::common_types.i32) {
				set_int(id, v.value.to_i32());
			} else if (t == kaba::common_types._bool) {
				check(id, v.value.to_bool());
			} else {
				set_string(id, str(v.value));
			}
		}
	}
}

void UserComponentPanel::on_edit() {
	auto e = data->entity(index);
	if (auto c = e->_get_component_generic_(type)) {
		auto desc = data->session->plugin_manager->describe_class(type, c);
		for (auto&& [i, v]: enumerate(desc.variables)) {
			const string id = format("var-%d", i);
			auto t = find_class_member_type(type, v.name);
			if (t->name == "Template*") {
				// ...
			} else if (t == kaba::common_types.f32) {
				v.value = get_float(id);
			} else if (t == kaba::common_types.i32) {
				v.value = get_int(id);
			} else if (t == kaba::common_types._bool) {
				v.value = is_checked(id);
			} else {
				v.value = get_string(id);
			}
		}

		editing = true;
		data->entity_edit_component(e, type, desc);
		editing = false;
	}
}
