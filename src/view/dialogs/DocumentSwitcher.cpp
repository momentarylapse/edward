//
// Created by Michael Ankele on 2025-04-10.
//

#include "DocumentSwitcher.h"
#include "../EdwardWindow.h"
#include "../DocumentSession.h"
#include "../../Session.h"
#include <lib/base/sort.h>
#include <lib/xhui/controls/ListView.h>


DocumentSwitcher::DocumentSwitcher(EdwardWindow* _editor_window) : Dialog("...", 400, 400, _editor_window, xhui::DialogFlags::NoHeader) {
	editor_window = _editor_window;
	session = editor_window->session.get();
	// sort by last usage
	documents = base::sorted(session->documents, [] (DocumentSession* a, DocumentSession* b) {
		return a->_last_usage_counter > b->_last_usage_counter;
	});

	add_control("ListView", "open files", 0, 0, "list");
	auto list = (xhui::ListView*)get_control("list");
	list->column_factories[0].f_create = [this] (const string& id) {
		return xhui::create_control("Label", "!markup", id);
	};

	for (auto doc: documents)
		add_string("list", doc->title(true));
	set_int("list", documents.find(session->cur_doc));
	next();
}

DocumentSwitcher::~DocumentSwitcher() {
	int i = get_int("list");
	if (documents.num > 0 and i >= 0)
		session->set_active_doc(documents[i]);
}

void DocumentSwitcher::next() {
	set_int("list", loop(get_int("list") + 1, 0, documents.num));
}

void DocumentSwitcher::previous() {
	set_int("list", loop(get_int("list") - 1, 0, documents.num));
}
