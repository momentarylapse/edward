//
// Created by Michael Ankele on 2025-04-10.
//

#include "DocumentSwitcher.h"
#include "../EdwardWindow.h"
#include "../DocumentSession.h"
#include "../../Session.h"


// TODO sort by last-active

DocumentSwitcher::DocumentSwitcher(EdwardWindow* _editor_window) : Dialog("...", 400, 400, _editor_window, xhui::DialogFlags::NoHeader) {
	editor_window = _editor_window;

	add_control("ListView", "open files", 0, 0, "list");

	for (auto doc: weak(editor_window->session->documents))
		add_string("list", doc->title());
	auto session = editor_window->session.get();
	set_int("list", weak(session->documents).find(session->cur_doc));
	next();
}

DocumentSwitcher::~DocumentSwitcher() {
	int i = get_int("list");
	editor_window->session->set_active_doc(weak(editor_window->session->documents)[i]);
}

void DocumentSwitcher::next() {
	set_int("list", loop(get_int("list") + 1, 0, editor_window->session->documents.num));
}

void DocumentSwitcher::previous() {
	set_int("list", loop(get_int("list") - 1, 0, editor_window->session->documents.num));
}
