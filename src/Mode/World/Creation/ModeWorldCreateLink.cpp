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

string link_type_canonical(LinkType t);

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
			message = _("select second object to link");
		}
	} else {
		if (multi_view->hover.type == MVD_WORLD_OBJECT) {
			objects.add(multi_view->hover.index);

			WorldLink l;
			//l.pos = multi_view->get_cursor();
			l.pos = (data->objects[objects[0]].pos + data->objects[objects[1]].pos) / 2;
			l.ang = v_0;
			l.object[0] = objects[0];
			l.object[1] = objects[1];
			l.type = (LinkType)dialog->get_int("type");
			data->links.add(l);
			data->notify();
			abort();
		}
	}
}

void ModeWorldCreateLink::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}


