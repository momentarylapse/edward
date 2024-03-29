/*
 * ModeWorldCreateObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateObject.h"
#include "../../../data/world/DataWorld.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../multiview/MultiView.h"

static Path LastObjectFilename;

ModeWorldCreateObject::ModeWorldCreateObject(ModeWorld *_parent) :
	ModeCreation<ModeWorld, DataWorld>("WorldCreateObject", _parent)
{
	message = _("select new object");
}

void ModeWorldCreateObject::on_start() {
	filename = LastObjectFilename;
	if (filename.is_empty())
		on_find_object();

	dialog = new hui::Panel();
	dialog->from_resource("world_new_object_dialog");
	dialog->set_string("kind", filename.str());
	dialog->enable("name", false);
	dialog->event("find_object", std::bind(&ModeWorldCreateObject::on_find_object, this));
	session->win->set_side_panel(dialog);

	if (!filename.is_empty())
		message = _("place new object");
}


void ModeWorldCreateObject::on_end() {
	session->win->set_side_panel(nullptr);
}

void ModeWorldCreateObject::on_find_object() {
	session->storage->file_dialog(FD_MODEL, false, true).then([this] (const auto& p) {
		filename = p.simple;
		LastObjectFilename = filename;
		message = _("place new object");
	});
}


void ModeWorldCreateObject::on_left_button_down() {
	if (!filename.is_empty()) {
		data->add_object(filename, multi_view->get_cursor());
		//Abort();
	}
}
