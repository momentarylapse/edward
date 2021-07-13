/*
 * ComponentSelectionDialog.cpp
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#include "ComponentSelectionDialog.h"
#include "../../../Storage/Storage.h"
#include "../../../lib/kaba/kaba.h"


// seems quick enough
Array<ScriptInstanceData> enumerate_components() {
	Array<ScriptInstanceData> r;
	auto files = dir_search(storage->root_dir_kind[FD_SCRIPT], "*.kaba", "rf");
	for (auto &f: files) {
		try {
			auto s = kaba::load(storage->root_dir_kind[FD_SCRIPT] << f, true);
			for (auto c: s->classes())
				if (c->is_derived_from_s("y.Component") and c->name != "Component")
					r.add({f, c->name});
		} catch (Exception &e) {
			msg_error(e.message());
		}
	}
	return r;
}


ComponentSelectionDialog::ComponentSelectionDialog(hui::Window *parent, ScriptInstanceData &_data) :
		hui::Dialog("component-selection-dialog", parent)
{
	selected = false;
	data = &_data;

	available = enumerate_components();
	for (auto &c: available)
		add_string("list", format("%s\\%s", c.class_name, c.filename));

	event_x("list", "hui:select", [=] {
		int row = get_int("list");
		enable("ok", row >= 0);
	});
	event_x("list", "hui:activate", [=] {
		int row = get_int("list");
		if (row >= 0) {
			selected = true;
			*data = available[row];
			request_destroy();
		}
	});
	event("cancel", [=] {
		request_destroy();
	});
	event("ok", [=] {
		int row = get_int("list");
		if (row >= 0) {
			selected = true;
			*data = available[row];
		}
		request_destroy();
	});
}


bool ComponentSelectionDialog::choose(hui::Window *parent, ScriptInstanceData &data) {
	auto dlg = ownify(new ComponentSelectionDialog(parent, data));
	dlg->run();
	return dlg->selected;
}
