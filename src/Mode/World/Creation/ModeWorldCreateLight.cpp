/*
 * ModeWorldCreateLight.cpp
 *
 *  Created on: Mar 8, 2020
 *      Author: michi
 */

#include "ModeWorldCreateLight.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"

ModeWorldCreateLight::ModeWorldCreateLight(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateObject", _parent)
{
	message = _("put new light");
}

void ModeWorldCreateLight::on_start() {
	dialog = new hui::Panel();
	dialog->add_list_view("Type", 0, 0, "type");
	dialog->add_string("type", "Directional");
	dialog->add_string("type", "Point source");
	dialog->set_int("type", 1);
	ed->set_side_panel(dialog);
}


void ModeWorldCreateLight::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeWorldCreateLight::on_left_button_down() {
	int t = dialog->get_int("type");
	WorldLight l;
	l.pos = multi_view->get_cursor();
	l.ang = v_0;
	l.col = White;
	l.enabled = true;
	l.harshness = 0.8f;
	l.type = (LightType)t;
	l.radius = multi_view->cam.radius;

	data->lights.add(l);
	data->notify();
}

void ModeWorldCreateLight::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}