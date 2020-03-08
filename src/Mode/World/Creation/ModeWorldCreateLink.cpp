/*
 * ModeWorldCreateLink.cpp
 *
 *  Created on: Mar 8, 2020
 *      Author: michi
 */

#include "ModeWorldCreateLink.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"

ModeWorldCreateLink::ModeWorldCreateLink(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateObject", _parent)
{
	message = _("select new link pivot point");
}

void ModeWorldCreateLink::on_start() {

	dialog = new hui::Panel();
	dialog->from_resource("world_new_object_dialog");
	dialog->set_string("kind", filename);
	dialog->enable("name", false);
	//dialog->event("find_object", std::bind(&ModeWorldCreateObject::on_find_object, this));
	ed->set_side_panel(dialog);
}


void ModeWorldCreateLink::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeWorldCreateLink::on_left_button_down() {
}

void ModeWorldCreateLink::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}


