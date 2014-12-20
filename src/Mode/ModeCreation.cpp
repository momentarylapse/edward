/*
 * ModeCreation.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeCreation.h"
#include "../Edward.h"
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

void ModeCreationBase::onDrawMeta()
{
	ModeBase::onDrawMeta();

	ed->drawStr(MaxX / 2, MaxY - 20, message);
}

void ModeCreationBase::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);
}

void ModeCreationBase::abort()
{
	assert(parent);
	ed->setMode(parent);
}

