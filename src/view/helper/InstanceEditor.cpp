//
// Created by michi on 7/7/26.
//

#include "InstanceEditor.h"
#include <storage/Storage.h>
#include <stuff/PluginManager.h>
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/kaba/kaba.h>
#include <lib/any/conversion.h>
#include <lib/math/vec3.h>


const kaba::Class* find_class_member_type(const kaba::Class* c, const string& name) {
	for (const auto& e: c->elements)
		if (e.name == name)
			return e.type;
	return nullptr;
}

InstanceEditor::InstanceEditor(Session* _session, const kaba::Class* _type, const Func& _f_on_edit) : Panel("") {
	session = _session;
	type = _type;
	f_on_edit = _f_on_edit;
	from_source(R"foodelim(
Dialog instance-editor-panel ''
	Grid grid-variables ''
)foodelim");
}

void InstanceEditor::build(void* data) {
	set_target("grid-variables");
	const auto desc = session->plugin_manager->describe_class(type, data);
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
				session->storage->file_dialog(FD_TEMPLATE, false, true).then([this] (const ComplexPath& p) {

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
		} else if (t == kaba::common_types.vec3) {
			add_control("Grid", "", 2, i, id + ":grid");
			set_target(id + ":grid");
			add_control("SpinButton", "", 0, 0, id + ":x");
			add_control("SpinButton", "", 0, 1, id + ":y");
			add_control("SpinButton", "", 0, 2, id + ":z");
			set_options(id + ":x", "range=::0.001");
			set_options(id + ":y", "range=::0.001");
			set_options(id + ":z", "range=::0.001");
			set_target("grid-variables");
			event(id + ":x", [this] {
				on_edit();
			});
			event(id + ":y", [this] {
				on_edit();
			});
			event(id + ":z", [this] {
				on_edit();
			});
		} else if (t->is_enum()) {
			add_control("ComboBox", "", 2, i, id);
			for (auto c: weak(t->constants))
				if (c->type == t)
					add_string(id, c->name);
		} else {
			add_control("Edit", "", 2, i, id);
		}
		event(id, [this] {
			on_edit();
		});
	}
	if (desc.variables.num == 0)
		add_control("Label", "no variables", 0, 0, "");
}

void InstanceEditor::update_ui(void* data) {
	description = session->plugin_manager->describe_class(type, data);
	for (const auto& [i, v]: enumerate(description.variables)) {
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
		} else if (t == kaba::common_types.vec3) {
			if (v.value.is_list() and v.value.as_list().num >= 3) {
				set_float(id + ":x", v.value.as_list()[0].to_f32());
				set_float(id + ":y", v.value.as_list()[1].to_f32());
				set_float(id + ":z", v.value.as_list()[2].to_f32());
			}
		} else if (t->is_enum()) {
			int n = 0;
			for (auto c: weak(t->constants))
				if (c->type == t) {
					if (v.value.to_i32() == c->as_int())
						set_int(id, n);
					n ++;
				}
		} else {
			set_string(id, str(v.value));
		}
	}
}

void InstanceEditor::on_edit() {
	for (auto&& [i, v]: enumerate(description.variables)) {
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
		} else if (t == kaba::common_types.vec3) {
			if (v.value.is_list() and v.value.as_list().num >= 3) {
				v.value = vec3_to_any(vec3(get_float(id + ":x"), get_float(id + ":y"), get_float(id + ":z")));
			}
		} else if (t->is_enum()) {
			int n = 0;
			int nn = get_int(id);
			for (auto c: weak(t->constants))
				if (c->type == t) {
					if (n == nn)
						v.value = c->as_int();
					n ++;
				}
		} else {
			v.value = get_string(id);
		}
	}
	f_on_edit(description);
}
