//
// Created by michi on 9/9/25.
//

#include "EditorWindow.h"
#include "DocumentEditor.h"
#include "DocumentSwitcher.h"
#include <lib/xhui/xhui.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/controls/MultilineEdit.h>
#include <lib/os/filesystem.h>

namespace codeedit {

EditorWindow::EditorWindow() : obs::Node<Window>("", 800, 600) {
	from_source(R"foodelim(
Window test 'test' padding=0
	TabControl tab 'a' bar=no
)foodelim");

#ifdef OS_MAC
	int mod = xhui::KEY_SUPER;
#else
	int mod = xhui::KEY_CONTROL;
#endif

#ifdef OS_MAC
	int key_next_doc = xhui::KEY_TAB + xhui::KEY_ALT;//xhui::KEY_CONTROL;
#else
	int key_next_doc = xhui::KEY_TAB + xhui::KEY_CONTROL;
#endif

	set_key_code("new", xhui::KEY_N + mod);
	set_key_code("open", xhui::KEY_O + mod);
	set_key_code("save", xhui::KEY_S + mod);
	set_key_code("save-as", xhui::KEY_S + mod + xhui::KEY_SHIFT);
	set_key_code("next-document", key_next_doc);
	set_key_code("previous-document", key_next_doc + xhui::KEY_SHIFT);


	event("new", [this] {
		create_document_editor();
	});
	event("open", [this] {
		xhui::FileSelectionDialog::ask(this, "open..", os::fs::current_directory(), {}).then([this] (const Path& filename) {
			auto e = create_document_editor();
			e->load(filename);
		});
		create_document_editor();
	});
	event("next-document", [this] {
		if (switcher) {
			switcher->next();
		} else {
			switcher = new DocumentSwitcher(this);
			open_dialog(switcher);
		}
	});
	event("previous-document", [this] {
		if (switcher) {
			switcher->previous();
		} else {
			switcher = new DocumentSwitcher(this);
			open_dialog(switcher);
		}
	});
}

void EditorWindow::on_key_up(int key_code) {
	int key = key_code & 0xff;
	if ((key == xhui::KEY_LCONTROL or key == xhui::KEY_LALT) and switcher) {
		switcher->request_destroy();
		switcher = nullptr;
	}
}


DocumentEditor* EditorWindow::create_document_editor() {
	static int counter = 0;
	string id = format("edit-%d", counter ++);

	set_target("tab");
	add_control("MultilineEdit", "", document_editors.num, 0, id);
	auto edit = (xhui::MultilineEdit*)get_control(id);
	edit->set_option("focusframe", "no");
	edit->set_option("monospace", "");
	edit->set_option("fontsize", "14");


	auto e = new DocumentEditor();
	e->create_controls(this, document_editors.num);
	document_editors.add(e);

	e->out_changed >> create_sink([this] {
		update_title();
	});


	set_active(e);

	return e;
}

void EditorWindow::set_active(DocumentEditor* editor) {
	active_editor = editor;
	int index = weak(document_editors).find(editor);
	set_int("tab", index);

	focus(editor->edit->id);
	update_title();
}

void EditorWindow::update_title() {
	if (active_editor)
		set_title(active_editor->title() + " - xedit");
}

}
