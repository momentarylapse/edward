/*
 * ComponentSelectionDialog.cpp
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#include "ComponentSelectionDialog.h"
#include "../../../Storage/Storage.h"
#include "../../../lib/kaba/kaba.h"
#include "../../../lib/os/filesystem.h"


// seems quick enough
Array<ScriptInstanceData> enumerate_components() {
	Array<ScriptInstanceData> r;
	auto files = os::fs::search(storage->root_dir_kind[FD_SCRIPT], "*.kaba", "rf");
	for (auto &f: files) {
		try {
			auto context = ownify(kaba::Context::create());
			auto s = context->load_module(storage->root_dir_kind[FD_SCRIPT] << f, true);
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


ComponentSelectionDialog::ComponentSelectionDialog(hui::Window *parent, Callback _on_select) :
		hui::Dialog("component-selection-dialog", parent)
{
	on_select = _on_select;

	available = enumerate_components();
	for (auto &c: available)
		add_string("list", format("%s\\%s", c.class_name, c.filename));

	event_x("list", "hui:select", [this] {
		int row = get_int("list");
		enable("ok", row >= 0);
	});
	event_x("list", "hui:activate", [this] {
		int row = get_int("list");
		if (row >= 0) {
			on_select(available[row]);
			request_destroy();
		}
	});
	event("cancel", [this] {
		request_destroy();
	});
	event("ok", [this] {
		int row = get_int("list");
		if (row >= 0)
			on_select(available[row]);
		request_destroy();
	});
}


void ComponentSelectionDialog::choose(hui::Window *parent, Callback on_select) {
	hui::fly(new ComponentSelectionDialog(parent, on_select));
}
