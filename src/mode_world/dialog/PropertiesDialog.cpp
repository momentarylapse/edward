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
}

void PropertiesDialog::fill() {
	set_color("bgc", temp.background_color);
	set_float("gravitation_x", temp.gravity.x);
	set_float("gravitation_y", temp.gravity.y);
	set_float("gravitation_z", temp.gravity.z);

	reset("skybox");
	for (const auto&& [i, sb]: enumerate(temp.skybox_files))
		add_string("skybox", format("%d\\%s", i, sb));

	reset("script_list");
	for (const auto& script: temp.scripts)
		add_string("script_list", str(script.filename));
}


void PropertiesDialog::apply() {
	data->execute(new ActionWorldEditData(temp));
}



