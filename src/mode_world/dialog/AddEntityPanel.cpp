//
// Created by Michael Ankele on 2025-02-04.
//

#include "AddEntityPanel.h"

#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

#include "../ModeWorld.h"

AddEntityPanel::AddEntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("add-entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog add-entity-panel ''
	Grid card-add '' class=card
		Group group-add 'Add'
			Grid ? ''
				ListView add-list 'a' nobar dragsource=entity cangrabfocus=no noexpandy height=250
				---|
				FileSelector add-files '' dragsource=entity-file expandy
)foodelim");
	/*size_mode_y = SizeMode::Shrink;
	min_width_user = 320;*/


	add_string("add-list", "Entity - empty");
	add_string("add-list", "Camera");
	add_string("add-list", "Light - directional");
	add_string("add-list", "Light - point");
	add_string("add-list", "Light - cone");
	add_string("add-list", "Terrain");

	set_options("add-files", "linkevents");
	set_options("add-files", "directory=" + str(mode_world->session->storage->root_dir));
	set_options("add-files", "filter=*.model");

	event_x("add-list", xhui::event_id::DragStart, [this] {
		int i = get_int("add-list");
		mode_world->session->win->start_drag("New entity", "add-entity-default-" + str(i));
	});
}
