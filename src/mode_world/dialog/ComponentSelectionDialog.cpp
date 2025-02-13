//
// Created by Michael Ankele on 2025-02-13.
//

#include "ComponentSelectionDialog.h"
#include <lib/base/sort.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>

Array<ScriptInstanceData> enumerate_classes(Session *session, const string& full_base_class);

ComponentSelectionDialog::ComponentSelectionDialog(xhui::Panel* parent, Session* session) : Dialog("component-selection-dialog", parent) {
	width = 400;
	height = 600;

	//add_control("ListView", "class\\file", 0, 0, "list");

	classes = enumerate_classes(session, "ecs.Component");
	classes = base::sorted(classes, [] (const ScriptInstanceData& a, const ScriptInstanceData& b) {
		return a.class_name <= b.class_name;
	});

	for (const auto& c: classes)
		add_string("list", format("%s\\%s", c.class_name, c.filename));

	event("list", [this] {
		int n = get_int("list");
		if (n >= 0 and n < classes.num) {
			promise(classes[n]);
			request_destroy();
		}
	});
	event(xhui::event_id::Close, [this] {
		promise.fail();
		request_destroy();
	});
}

base::future<ScriptInstanceData> ComponentSelectionDialog::ask(xhui::Panel* parent, Session* session) {
	auto dlg = new ComponentSelectionDialog(parent, session);
	parent->open_dialog(dlg);
	return dlg->promise.get_future();
}



