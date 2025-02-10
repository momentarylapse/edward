//
// Created by Michael Ankele on 2025-02-10.
//

#include "PropertiesDialog.h"
#include <mode_world/data/DataWorld.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <mode_world/action/ActionWorldEditData.h>

PropertiesDialog::PropertiesDialog(xhui::Panel* parent, DataWorld* _data) : Dialog("world_dialog", parent) {
	data = _data;
	temp = data->meta_data;

	event(xhui::event_id::Close, [this] {
		request_destroy();
	});

	set_color("bgc", temp.background_color);
	set_float("gravitation_x", temp.gravity.x);
	set_float("gravitation_y", temp.gravity.y);
	set_float("gravitation_z", temp.gravity.z);

	for (const auto&& [i, sb]: enumerate(temp.skybox_files))
		add_string("skybox", format("%d\\%s", i, sb));
	for (const auto& script: temp.scripts)
		add_string("script_list", str(script.filename));

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

void PropertiesDialog::apply() {
	data->execute(new ActionWorldEditData(temp));
}



