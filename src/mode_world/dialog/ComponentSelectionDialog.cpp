//
// Created by Michael Ankele on 2025-02-13.
//

#include "ComponentSelectionDialog.h"
#include <lib/base/sort.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <stuff/PluginManager.h>

#include "lib/kaba/Module.h"
#include "lib/kaba/syntax/Class.h"
#include "lib/kaba/syntax/SyntaxTree.h"

ComponentSelectionDialog::ComponentSelectionDialog(xhui::Panel* parent, Session* session, const string& base_class) : Dialog("component-selection-dialog", parent) {
	width = 400;
	height = 600;

	classes = session->plugin_manager->enumerate_classes(base_class);
	classes = base::sorted(classes, [] (const kaba::Class* a, const kaba::Class* b) {
		return a->name <= b->name;
	});

	for (const auto c: classes) {
		auto s = session->plugin_manager->describe_class(c);
		add_string("list", format("%s\\%s", c->name, s.filename));
	}

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

base::future<const kaba::Class*> ComponentSelectionDialog::ask(xhui::Panel* parent, Session* session, const string& base_class) {
	auto dlg = new ComponentSelectionDialog(parent, session, base_class);
	parent->open_dialog(dlg);
	return dlg->promise.get_future();
}



