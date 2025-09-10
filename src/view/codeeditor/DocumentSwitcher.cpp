//
// Created by Michael Ankele on 2025-04-10.
//

#include "DocumentSwitcher.h"
#include "EditorWindow.h"
#include "DocumentEditor.h"
#include <lib/os/msg.h>

namespace codeedit {

// TODO sort by last-active

DocumentSwitcher::DocumentSwitcher(EditorWindow* _editor_window) : Dialog("...", 400, 400, _editor_window, xhui::DialogFlags::NoHeader) {
	editor_window = _editor_window;

	add_control("ListView", "open files", 0, 0, "list");

	for (auto e: weak(editor_window->document_editors))
		add_string("list", e->filename.basename());
	set_int("list", weak(editor_window->document_editors).find(editor_window->active_editor));
	next();
}

DocumentSwitcher::~DocumentSwitcher() {
	int i = get_int("list");
	editor_window->set_active(weak(editor_window->document_editors)[i]);
}

void DocumentSwitcher::next() {
	set_int("list", loop(get_int("list") + 1, 0, editor_window->document_editors.num));
}

void DocumentSwitcher::previous() {
	set_int("list", loop(get_int("list") - 1, 0, editor_window->document_editors.num));
}

}
