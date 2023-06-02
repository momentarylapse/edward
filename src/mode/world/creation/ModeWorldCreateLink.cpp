/*
 * ModeWorldCreateLink.cpp
 *
 *  Created on: Mar 8, 2020
 *      Author: michi
 */

#include "ModeWorldCreateLink.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldLink.h"
#include "../../../data/world/WorldObject.h"
#include "../../../Edward.h"
#include "../../../multiview/MultiView.h"
#include "../../../action/world/link/ActionWorldAddLink.h"

ModeWorldCreateLink::ModeWorldCreateLink(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateObject", _parent)
{
	message = _("select first object to link");
}

void ModeWorldCreateLink::on_start() {

	dialog = new hui::Panel();
	dialog->add_list_view("Type", 0, 0, "type");
	for (int i=0; i<4; i++)
		dialog->add_string("type", link_type_canonical((LinkType)i));
	dialog->set_int("type", 0);
	ed->set_side_panel(dialog);
}


void ModeWorldCreateLink::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeWorldCreateLink::on_left_button_down() {

	if (objects.num == 0) {
		if (multi_view->hover.type == MVD_WORLD_OBJECT) {
			objects.add(multi_view->hover.index);
			message = _("select second object or background (= fixed)");
		}
	} else {
		if (multi_view->hover.type == MVD_WORLD_OBJECT) {
			objects.add(multi_view->hover.index);

			WorldLink l;
			l.pos = (data->objects[objects[0]].pos + data->objects[objects[1]].pos) / 2;
			l.object[0] = objects[0];
			l.object[1] = objects[1];
			l.type = (LinkType)dialog->get_int("type");
			data->execute(new ActionWorldAddLink(l));
			abort();
		} else if (multi_view->hover.index < 0) {

			WorldLink l;
			l.pos = multi_view->get_cursor();
			l.object[0] = objects[0];
			l.object[1] = -1;
			l.type = (LinkType)dialog->get_int("type");
			data->execute(new ActionWorldAddLink(l));
			abort();
		}
	}
}

void ModeWorldCreateLink::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}


