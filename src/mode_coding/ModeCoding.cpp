//
// Created by michi on 9/21/25.
//

#include "ModeCoding.h"
#include <view/DocumentSession.h>
#include <view/codeeditor/CodeEditor.h>
#include <data/Data.h>
#include <storage/format/Format.h>
#include <lib/xhui/Panel.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/controls/MenuBar.h>
#include <lib/xhui/Resource.h>
#include <lib/os/file.h>
#include <Session.h>
#include <view/EdwardWindow.h>

#include "lib/os/msg.h"

class CodingPanel : public xhui::Panel {
public:
	DocumentSession* doc;
	codeedit::CodeEditor* editor;

	explicit CodingPanel(DocumentSession* _doc) : xhui::Panel("") {
		doc = _doc;
		from_source(R"foodelim(
Dialog coding-panel ''
	MultilineEdit edit grabfocus monospace linenumbers focusframe=no
)foodelim");
		propagate_events = true;
		editor = new codeedit::CodeEditor(this, "edit");
	}
	void load(const Path& filename) {
		editor->load(filename);
	}
};

CodeData::CodeData(DocumentSession* doc) : Data(doc, FD_SCRIPT) {
}


ModeCoding::ModeCoding(DocumentSession* doc) : Mode(doc) {
	coding_panel = new CodingPanel(doc);
	doc->set_document_panel(coding_panel);
	data = new CodeData(doc);
	data->reset();
	generic_data = data.get();
}

ModeCoding::~ModeCoding() = default;

void ModeCoding::on_enter_rec() {
	/*session->out_changed >> create_sink([this] {
		update_menu();
	});

	auto win = session->win;
	auto tb = win->toolbar;
	tb->set_by_id("model-toolbar");
	win->enable("mode_model_animation", false);

	event_ids_rec.add(win->event("mode_model_mesh", [this] {
		doc->set_mode(mode_mesh.get());
	}));
	event_ids_rec.add(win->event("mode_model_skeleton", [this] {
		doc->set_mode(mode_skeleton.get());
	}));
	event_ids_rec.add(session->win->event("mode_properties", [this] {
		session->win->open_dialog(new ModelPropertiesDialog(session->win, data.get()));
	}));

	event_ids_rec.add(session->win->event("save", [this] {
		if (data->filename.is_empty())
			session->storage->save_as(data.get());
		else
			session->storage->save(data->filename, data.get());
	}));
	event_ids_rec.add(session->win->event("save-as", [this] {
		session->storage->save_as(data.get());
	}));*/
}

void ModeCoding::on_leave_rec() {
	/*session->out_changed.unsubscribe(this);

	for (int uid: event_ids_rec)
		session->win->remove_event_handler(uid);
	event_ids_rec.clear();*/
}

void ModeCoding::on_command(const string& id) {
	if (id == "new")
		session->universal_new(FD_SCRIPT);
	if (id == "open")
		session->universal_open(FD_SCRIPT);
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}


void ModeCoding::on_set_menu() {
	auto tb = session->win->tool_bar;
	tb->set_by_id("coding-toolbar");


	auto menu = xhui::create_resource_menu("menu-coding");
	session->win->menu_bar->set_menu(menu);
}

void ModeCoding::update_menu() {
}

void ModeCoding::load(const Path& filename) {
	coding_panel->load(filename);
	data->filename = filename;
}


