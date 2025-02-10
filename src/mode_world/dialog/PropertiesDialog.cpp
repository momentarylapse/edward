//
// Created by Michael Ankele on 2025-02-10.
//

#include "PropertiesDialog.h"
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <mode_world/data/DataWorld.h>

#include "../../../../lib/base/iter.h"

PropertiesDialog::PropertiesDialog(xhui::Panel* parent, DataWorld* _data) : Dialog("world_dialog", parent) {
	data = _data;

	event(xhui::event_id::Close, [this] {
		msg_write("AAAAAA");
		request_destroy();
	});

	set_color("bgc", data->meta_data.background_color);
	set_float("gravitation_x", data->meta_data.gravity.x);
	set_float("gravitation_y", data->meta_data.gravity.y);
	set_float("gravitation_z", data->meta_data.gravity.z);

	for (const auto&& [i, sb]: enumerate(data->meta_data.skybox_files))
		add_string("skybox", format("%d\\%s", i, sb));
	for (const auto& script: data->meta_data.scripts)
		add_string("script_list", str(script.filename));

	event("bgc", [this] {
		data->meta_data.background_color = get_color("bgc");
		request_redraw();
	});
}


