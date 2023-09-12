/*
 * ModeCreation.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeCreation.h"
#include "../Session.h"
#include "../multiview/DrawingHelper.h"
#include "../lib/nix/nix.h"
#include <assert.h>

ModeCreationBase::ModeCreationBase(const string &_name, ModeBase *_parent) :
	ModeBase(_parent->session, _name, _parent, _parent->multi_view, "")
{
	// don't nest creation modes!
	if (dynamic_cast<ModeCreationBase*>(parent_untyped))
		parent_untyped = parent_untyped->parent_untyped;

	assert(parent_untyped);
	menu_id = parent_untyped->menu_id;
	dialog = NULL;
}

void ModeCreationBase::on_draw() {
	//ModeBase::onDraw();
	on_draw_post();

	nix::set_shader(session->gl->default_2d.get());
	session->drawing_helper->draw_str(nix::target_width / 2, nix::target_height - 20, message);
}

void ModeCreationBase::on_draw_win(MultiView::Window *win) {
	parent_untyped->on_draw_win(win);
}

void ModeCreationBase::on_set_multi_view() {
	parent_untyped->on_set_multi_view();
}

void ModeCreationBase::abort() {
	assert(parent_untyped);
	hui::run_later(0.01f, [this]{ session->set_mode(parent_untyped); });
}

