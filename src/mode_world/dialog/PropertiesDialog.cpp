//
// Created by Michael Ankele on 2025-02-10.
//

#include "PropertiesDialog.h"
#include <mode_world/data/DataWorld.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <lib/xhui/Menu.h>
#include <lib/xhui/controls/ListView.h>
#include <mode_world/action/ActionWorldEditData.h>
#include <storage/Storage.h>
#include <view/dialogs/CommonDialogs.h>
#include <world/World.h>
#include <y/BaseClass.h>

#include "ComponentSelectionDialog.h"
#include <stuff/PluginManager.h>

class SystemPanel : public obs::Node<xhui::Panel> {
public:
	explicit SystemPanel(PropertiesDialog* _properties_dialog) : obs::Node<xhui::Panel>("") {
		properties_dialog = _properties_dialog;
		data = properties_dialog->data;
		from_source(R"foodelim(
Dialog system 'System'
	Grid ? '' class=card
		Grid ? ''
			Label class '' bold
			Label filename '' small
		---|
		Expander contents '' expandx
			Grid ? ''
				Grid variables ''
				---|
				Grid ? ''
					Label ? '' expandx
					Button edit 'Edit code' primary noexpandx
					Button delete 'Delete' danger noexpandx
)foodelim");
		event("delete", [this] {
			if (index >= 0) {
				properties_dialog->temp = data->meta_data;
				properties_dialog->temp.systems.erase(index);
				properties_dialog->apply();
			}
		});
		event("edit", [this] {
			data->session->universal_edit(FD_SCRIPT, data->meta_data.systems[index].filename, true);
		});
	}
	void update(int _index) {
		index = _index;
		auto& s = data->meta_data.systems[index];
		set_string("class", s.class_name);
		set_string("filename", str(s.filename));
		set_target("variables");
		for (const auto& [i, v]: enumerate(s.variables)) {
			string id_var = format("var-%d", i);
			add_control("Label", v.name, 0, i, format("l-var-%d", i));
			add_control("Edit", v.value, 1, i, id_var);
			event(id_var, [this, i=i, id=id_var] {
				properties_dialog->temp = data->meta_data;
				properties_dialog->temp.systems[index].variables[i].value = get_string(id);
				properties_dialog->apply();
			});
		}
		if (s.variables.num == 0)
			add_control("Label", "no variables", 0, 0, "");
	}
	void set_selected(bool selected) {
		expand("contents", selected);
	}
	PropertiesDialog* properties_dialog;
	DataWorld* data;
	int index = -1;
};

PropertiesDialog::PropertiesDialog(DataWorld* _data) : Node<xhui::Panel>("") {//: Dialog("world_dialog", parent) {
	data = _data;
	temp = data->meta_data;

	from_resource("world_dialog");

	auto systems_list = (xhui::ListView*)get_control("systems");
	systems_list->column_factories[0].f_create = [this](const string& id) -> xhui::Control* {
		return new SystemPanel(this);
	};
	systems_list->column_factories[0].f_set = [this](xhui::Control* c, const string& t) {
		reinterpret_cast<SystemPanel*>(c)->update(t._int());
	};
	systems_list->column_factories[0].f_select = [this](xhui::Control* c, bool selected) {
		reinterpret_cast<SystemPanel*>(c)->set_selected(selected);
	};

	data->out_changed >> create_sink([this] {
		temp = data->meta_data;
		fill();
	});

	/*event(xhui::event_id::Close, [this] {
		//request_destroy();
	});*/

	fill();

	event_x("skybox", xhui::event_id::RightButtonDown, [this] {
		auto m = new xhui::Menu;
		m->add_item("skybox-delete", "Delete");
		m->add_item("skybox-choose", "Choose file...");
		m->add_item("skybox-add", "Add file...");
		m->open_popup(this);
	});
	event("skybox-delete", [this] {
		int n = get_int("skybox");
		if (n >= 0)
			temp.skybox_files.erase(n);
		apply();
	});
	event("skybox-add", [this] {
		data->session->storage->file_dialog(FD_MODEL, false, true).then([this] (const ComplexPath& p) {
			temp.skybox_files.add(p.simple);
			apply();
		});
	});

	event("bgc", [this] {
		temp.background_color = get_color("bgc");
		apply();
	});
	event("physics_enabled", [this] {
		temp.physics_enabled = is_checked("physics_enabled");
		apply();
	});
	event("physics-mode", [this] {
		int n = get_int("physics-mode");
		if (n == 0)
			temp.physics_mode = PhysicsMode::SIMPLE;
		else if (n == 1)
			temp.physics_mode = PhysicsMode::FULL_EXTERNAL;
		apply();
	});
	event("gravitation_x", [this] {
		temp.gravity.x = get_float("gravitation_x");
		apply();
	});
	event("gravitation_y", [this] {
		temp.gravity.y = get_float("gravitation_y");
		apply();
	});
	event("gravitation_z", [this] {
		temp.gravity.z = get_float("gravitation_z");
		apply();
	});
	event_x("systems", xhui::event_id::RightButtonDown, [this] {
		auto m = new xhui::Menu;
		m->add_item("delete-system", "Delete");
		//m->add_item("system-choose", "Choose file...");
		m->add_item("add-system", "Add from file...");
		m->add_item("create-system", "Create new...");
		m->open_popup(this);
	});
	event("delete-system", [this] {
		int n = get_int("systems");
		if (n >= 0)
			temp.systems.erase(n);
		apply();
	});
	event("add-system", [this] {
		ComponentSelectionDialog::ask(this, data->session, "ui.Controller").then([this] (const kaba::Class* c) {
			temp.systems.add(data->session->plugin_manager->describe_class(c));
			apply();
		});
	});
	event("create-system", [this] {
		add_new_system();
	});
}

void PropertiesDialog::add_new_system() {
	data->session->storage->file_dialog(FD_SCRIPT, true, true).then([this] (const ComplexPath& path) {
		TextDialog::ask(this, "System name", "Test").then([this, path] (const string& name) {
			os::fs::write_text(path.complete, string(R"foodelim(use y.*

class <NAME> extends Controller
	var some_variable: f32

	func override on_init()

	func override on_iterate(dt: f32)
)foodelim").replace("<NAME>", name));

			temp.systems.add({name, path.relative, {}});
			apply();
		});
	});
}


void PropertiesDialog::fill() {
	set_color("bgc", temp.background_color);
	set_float("gravitation_x", temp.gravity.x);
	set_float("gravitation_y", temp.gravity.y);
	set_float("gravitation_z", temp.gravity.z);

	check("physics_enabled", temp.physics_enabled);
	if (temp.physics_mode == PhysicsMode::SIMPLE)
		set_int("physics-mode", 0);
	else if (temp.physics_mode == PhysicsMode::FULL_EXTERNAL)
		set_int("physics-mode", 1);

	reset("skybox");
	for (const auto&& [i, sb]: enumerate(temp.skybox_files))
		add_string("skybox", format("%d\\%s", i, sb));

	reset("systems");
	for (int i=0; i<temp.systems.num; i++)
		add_string("systems", str(i));
}


void PropertiesDialog::apply() {
	data->execute(new ActionWorldEditData(temp));
}



