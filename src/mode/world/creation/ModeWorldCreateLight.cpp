/*
 * ModeWorldCreateLight.cpp
 *
 *  Created on: Mar 8, 2020
 *      Author: michi
 */

#include "ModeWorldCreateLight.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldLight.h"
#include "../../../Edward.h"
#include "../../../multiview/MultiView.h"
#include "../../../action/world/light/ActionWorldAddLight.h"

ModeWorldCreateLight::ModeWorldCreateLight(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateObject", _parent)
{
	message = _("put new light");
}

void ModeWorldCreateLight::on_start() {
	dialog = new hui::Panel();
	dialog->add_grid("", 0, 0, "main");
	dialog->set_target("main");
	dialog->add_label(_("!big,bold,center\\New light source"), 0, 0, "");
	dialog->add_list_view("!style=navigation-sidebar\\Type", 0, 1, "type");
	dialog->add_string("type", "Directional");
	dialog->add_string("type", "Point source");
	dialog->add_string("type", "Cone");
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
	l.theta = (l.type == LightType::CONE) ? 0.7f : 0;

	data->execute(new ActionWorldAddLight(l));
}

void ModeWorldCreateLight::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}
