//
// Created by Michael Ankele on 2025-01-26.
//

#include "FileSelectionDialog.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"
#include "../controls/FileSelector.h"

namespace xhui {

/*
 *	Grid
 *		FileSelector
 *		Grid
 *			Label ""
 *			Button cancel
 *			Button ok
 */
FileSelectionDialog::FileSelectionDialog(Panel* parent, const string& title, const Path& dir, const Array<string>& params) : Dialog(title, 800, 600, parent) {

	saving = false;
	selector = new FileSelector("files");
	for (const auto& o: params)
		if (o.head(7) == "filter=")
			selector->set_filter(o.sub(7));
	selector->set_directory(dir);

	auto g1 = new Grid("grid1");
	add_child(g1);
	g1->add_child(selector, 0, 0);

	auto g3 = new Grid("grid3");
	g1->add_child(g3, 0, 1);
	auto spacer = new Label("spacer", "");
	g3->add_child(spacer, 0, 0);
	spacer->size_mode_x = SizeMode::Expand;
	auto button_cancel = new Button("cancel", "Cancel");
	button_cancel->min_width_user = 100;
	button_cancel->size_mode_x = SizeMode::Shrink;
	g3->add_child(button_cancel, 1, 0);
	auto button_ok = new Button("ok", "Ok");
	button_ok->min_width_user = 100;
	button_ok->size_mode_x = SizeMode::Shrink;
	button_ok->enable(false);
	g3->add_child(button_ok, 2, 0);

	selector->link_events();

	event("ok", [this] {
		promise(selector->get_selected_filename());
		request_destroy();
	});
	event("cancel", [this] {
		promise.fail();
		request_destroy();
	});
	event_x(selector->id, event_id::Select, [this] {
		enable("ok", !selector->get_selected_filename().is_empty());
	});
	event_x(selector->id, event_id::Activate, [this] {
		promise(selector->get_selected_filename());
		request_destroy();
	});
}

base::future<Path> FileSelectionDialog::ask(Panel* parent, const string& title, const Path& dir, const Array<string>& params) {
	auto dlg = new FileSelectionDialog(parent, title, dir, params);
	parent->open_dialog(dlg);
	return dlg->promise.get_future();
}



} // xhui