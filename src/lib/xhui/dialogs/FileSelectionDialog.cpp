//
// Created by Michael Ankele on 2025-01-26.
//

#include "FileSelectionDialog.h"

#include <lib/os/file.h>
#include <lib/os/filesystem.h>

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
FileSelectionDialog::FileSelectionDialog(Panel* parent, const string& title, const Path& dir, const Array<string>& params) : Dialog(title, 800, 600, parent, DialogFlags::CloseByEscape | DialogFlags::CloseByClickOutside) {

	saving = sa_contains(params, "save");

	if (saving) {
		from_source(R"foodelim(
Dialog xxx ''
	Grid ? ''
		FileSelector files ''
		---|
		Edit filename ''
		---|
		Grid ? ''
			Label ? '' expandx
			Button cancel 'Cancel' width=70
			Button ok 'Save' width=70 disabled default
)foodelim");
	} else {
		from_source(R"foodelim(
Dialog xxx ''
	Grid ? ''
		FileSelector files ''
		---|
		Grid ? ''
			Label ? '' expandx
			Button cancel 'Cancel' width=70
			Button ok 'Open' width=70 disabled default
)foodelim");
	}

	selector = (FileSelector*)get_control("files");
	for (const auto& o: params)
		if (o.head(7) == "filter=")
			selector->set_filter(o.sub(7));
	selector->set_directory(dir);
	selector->link_events();

	event("ok", [this] {
		answer = selected_path();
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event("filename", [this] {
		enable("ok", get_string("filename").num > 0);
	});
	event_x(selector->id, event_id::Select, [this] {
		const auto filename = selector->get_selected_filename();
		if (!filename.is_empty())
			if (!os::fs::is_directory(filename))
				set_string("filename", filename.basename());
		enable("ok", !filename.is_empty());
	});
	event_x(selector->id, event_id::Activate, [this] {
		answer = selector->get_selected_filename();
		request_destroy();
	});
}

Path FileSelectionDialog::selected_path() const {
	if (saving)
		// TODO add filter extension
		return selector->current_directory() | get_string("filename");
	return selector->get_selected_filename();
}


base::future<Path> FileSelectionDialog::ask(Panel* parent, const string& title, const Path& dir, const Array<string>& params) {
	auto dlg = new FileSelectionDialog(parent, title, dir, params);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}



} // xhui