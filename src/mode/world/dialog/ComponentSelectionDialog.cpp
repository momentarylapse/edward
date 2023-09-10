/*
 * ComponentSelectionDialog.cpp
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#include "ComponentSelectionDialog.h"
#include "../../../EdwardWindow.h"
#include "../../../storage/Storage.h"
#include "../../../lib/kaba/kaba.h"
#include "../../../lib/os/filesystem.h"


// seems quick enough
Array<ScriptInstanceData> enumerate_components(EdwardWindow *ed) {
	Array<ScriptInstanceData> r;
	auto files = os::fs::search(ed->storage->root_dir_kind[FD_SCRIPT], "*.kaba", "rf");
	for (auto &f: files) {
		try {
			auto context = ownify(kaba::Context::create());
			auto s = context->load_module(ed->storage->root_dir_kind[FD_SCRIPT] | f, true);
			for (auto c: s->classes()) {
				if (c->is_derived_from_s("ecs.Component") and c->name != "Component")
					r.add({f, c->name});
			}
		} catch (Exception &e) {
			msg_error(e.message());
		}
	}
	return r;
}


ComponentSelectionDialog::ComponentSelectionDialog(EdwardWindow *ed, hui::Window *parent) :
		hui::Dialog("component-selection-dialog", parent)
{
	available = enumerate_components(ed);
	for (auto &c: available)
		add_string("list", format("%s\\%s", c.class_name, c.filename));

	event_x("list", "hui:select", [this] {
		int row = get_int("list");
		enable("ok", row >= 0);
	});
	event_x("list", "hui:activate", [this] {
		int row = get_int("list");
		if (row >= 0) {
			promise(available[row]);
			request_destroy();
		}
	});
	event("cancel", [this] {
		promise.fail();
		request_destroy();
	});
	event("ok", [this] {
		int row = get_int("list");
		if (row >= 0)
			promise(available[row]);
		request_destroy();
	});
}


base::future<ScriptInstanceData> ComponentSelectionDialog::choose(EdwardWindow *ed, hui::Window *parent) {
	auto dlg = new ComponentSelectionDialog(ed, parent);
	hui::fly(dlg);
	return dlg->promise.get_future();
}
