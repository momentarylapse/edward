//
// Created by Michael Ankele on 2025-02-10.
//

#include "PropertiesDialog.h"
#include <mode_world/data/DataWorld.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <lib/xhui/Menu.h>
#include <mode_world/action/ActionWorldEditData.h>
#include <storage/Storage.h>
#include <view/dialogs/CommonDialogs.h>
#include <world/World.h>

#include "ComponentSelectionDialog.h"

PropertiesDialog::PropertiesDialog(xhui::Panel* parent, DataWorld* _data) : Dialog("world_dialog", parent) {
	data = _data;
	temp = data->meta_data;

	event(xhui::event_id::Close, [this] {
		request_destroy();
	});

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
		fill();
	});
	event("skybox-add", [this] {
		data->session->storage->file_dialog(FD_MODEL, false, true).then([this] (const ComplexPath& p) {
			temp.skybox_files.add(p.simple);
			apply();
			fill();
		});
	});

	event("bgc", [this] {
		temp.background_color = get_color("bgc");
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
	event_x("script_list", xhui::event_id::RightButtonDown, [this] {
		auto m = new xhui::Menu;
		m->add_item("system-delete", "Delete");
		//m->add_item("system-choose", "Choose file...");
		m->add_item("system-add", "Add from file...");
		m->add_item("system-create", "Create new...");
		m->open_popup(this);
	});
	event("system-delete", [this] {
		int n = get_int("script_list");
		if (n >= 0)
			temp.systems.erase(n);
		apply();
		fill();
	});
	event("system-add", [this] {
		ComponentSelectionDialog::ask(this, data->session, "ui.Controller").then([this] (const ScriptInstanceData& c) {
			temp.systems.add(c);
			apply();
			fill();
		});
	});
	event("system-create", [this] {
		data->session->storage->file_dialog(FD_SCRIPT, true, true).then([this] (const ComplexPath& path) {
			TextDialog::ask(this, "System name", "Test").then([this, path] (const string& name) {
				os::fs::write_text(path.complete, string(R"foodelim(use y.*

class <NAME> extends Controller
	var some_variable: f32

	func override on_init()

	func override on_iterate(dt: f32)
)foodelim").replace("<NAME>", name));

				temp.systems.add({path.relative, name});
				apply();
				fill();
			});
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
		set_int("physics_mode", 0);
	else if (temp.physics_mode == PhysicsMode::FULL_EXTERNAL)
		set_int("physics_mode", 1);

	reset("skybox");
	for (const auto&& [i, sb]: enumerate(temp.skybox_files))
		add_string("skybox", format("%d\\%s", i, sb));

	reset("script_list");
	for (const auto& s: temp.systems)
		add_string("script_list", format("%s - %s", s.class_name, s.filename));
}


void PropertiesDialog::apply() {
	data->execute(new ActionWorldEditData(temp));
}



