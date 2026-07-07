//
// Created by Michael Ankele on 2025-02-10.
//

#include "PropertiesDialog.h"
#include <mode_world/data/DataWorld.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <lib/xhui/Menu.h>
#include <mode_world/action/ActionWorldEditData.h>
#include <storage/Storage.h>
#include <world/World.h>
#include <world/systems/Physics.h>


PropertiesDialog::PropertiesDialog(DataWorld* _data) : Node<xhui::Panel>("") {//: Dialog("world_dialog", parent) {
	data = _data;
	temp = data->meta_data;

	from_resource("world_dialog");
	size_mode_x = SizeMode::Expand;

	data->out_changed >> create_sink([this] {
		if (!editing) {
			temp = data->meta_data;
			update_ui();
		}
	});

	update_ui();

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
	event("fog-enabled", [this] {
		temp.fog.enabled = is_checked("fog-enabled");
		apply();
	});
	event("fog-distance", [this] {
		temp.fog.density = 1 / get_float("fog-distance");
		apply();
	});
	event("fog-color", [this] {
		temp.fog.col = get_color("fog-color");
		apply();
	});
}


void PropertiesDialog::update_ui() {
	set_color("bgc", temp.background_color);
	reset("skybox");
	for (const auto&& [i, sb]: enumerate(temp.skybox_files))
		add_string("skybox", format("%d\\%s", i, sb));

	check("fog-enabled", temp.fog.enabled);
	set_float("fog-distance", 1/temp.fog.density);
	set_color("fog-color", temp.fog.col);
}


void PropertiesDialog::apply() {
	editing = true;
	temp.systems = data->meta_data.systems;
	data->execute(new ActionWorldEditData(temp));
	editing = false;
}



