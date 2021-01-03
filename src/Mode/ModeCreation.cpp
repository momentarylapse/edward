/*
 * ModeCreation.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeCreation.h"
#include "../Edward.h"
#include "../MultiView/DrawingHelper.h"
#include "../lib/nix/nix.h"
#include <assert.h>

ModeCreationBase::ModeCreationBase(const string &_name, ModeBase *_parent) :
	ModeBase(_name, _parent, _parent->multi_view, "")
{
	// don't nest creation modes!
	if (dynamic_cast<ModeCreationBase*>(parent))
		parent = parent->parent;

	assert(parent);
	menu_id = parent->menu_id;
	dialog = NULL;
}

void ModeCreationBase::on_draw() {
	//ModeBase::onDraw();
	on_draw_post();

	nix::SetShader(nix::Shader::default_2d);
	draw_str(nix::target_width / 2, nix::target_height - 20, message);
}

void ModeCreationBase::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}

void ModeCreationBase::on_set_multi_view() {
	parent->on_set_multi_view();
}

void ModeCreationBase::abort() {
	assert(parent);
	hui::RunLater(0.01f, [=]{ ed->set_mode(parent); });
}

