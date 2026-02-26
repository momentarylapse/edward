//
// Created by michi on 9/21/25.
//

#include "ModeCoding.h"
#include <view/DocumentSession.h>
#include <data/Data.h>
#include <storage/format/Format.h>
#include <storage/Storage.h>
#include <lib/codeeditor/CodeEditor.h>
#include <lib/xhui/Panel.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/controls/MenuBar.h>
#include <lib/xhui/Resource.h>
#include <lib/os/file.h>
#include <Session.h>
#include <view/EdwardWindow.h>

#include "lib/os/msg.h"

CodeData::CodeData(DocumentSession* doc) : Data(doc, FD_SCRIPT) {
}


ModeCoding::ModeCoding(DocumentSession* doc) : Mode(doc) {
	editor = new codeedit::CodeEditor();
	doc->set_document_panel(editor);
	data = new CodeData(doc);
	data->reset();
	generic_data = data.get();

	editor->out_changed >> create_sink([this] {
		data->out_changed();
	});
	editor->out_no_error >> create_sink([this] {
		session->info("no error");
	});
	editor->out_error >> create_data_sink<string>([this] (const string& msg) {
		session->error(msg);
	});
}

ModeCoding::~ModeCoding() = default;

void ModeCoding::on_enter_rec() {
}

void ModeCoding::on_connect_events_rec() {
	doc->event("compile", [this] {
		editor->show_errors();
	});
}

void ModeCoding::on_leave_rec() {
}

void ModeCoding::on_command(const string& id) {
	if (id == "new")
		session->universal_new(FD_SCRIPT);
	if (id == "open")
		session->universal_open(FD_SCRIPT);
	if (id == "save") {
		editor->save(get_filename());
		//session->storage->save(get_filename(), data.get());
	}
	if (id == "undo")
		editor->undo();
	if (id == "redo")
		editor->redo();
}


void ModeCoding::on_set_menu() {
	auto tb = session->win->tool_bar;
	tb->set_by_id("coding-toolbar");


	auto menu = xhui::create_resource_menu("menu-coding");
	session->win->menu_bar->set_menu(menu);
}

void ModeCoding::update_menu() {
}

void ModeCoding::load(const Path& _filename) {
	auto filename = _filename.absolute().canonical();
	session->storage->guess_root_directory(filename);
	editor->load(filename);
	data->filename = filename;
	data->reset_history();
	data->out_changed();
}

bool ModeCoding::is_save_state() const {
	return editor->is_save_state();
}

bool ModeCoding::is_undoable() const {
	return editor->is_undoable();
}

bool ModeCoding::is_redoable() const {
	return editor->is_redoable();
}


