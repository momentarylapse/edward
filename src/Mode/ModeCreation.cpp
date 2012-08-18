/*
 * ModeCreation.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeCreation.h"
#include "../Edward.h"
#include <assert.h>

ModeCreation::ModeCreation(const string &_name, Mode *_parent) :
	Mode(_name, _parent, _parent->data_generic, _parent->multi_view, "")
{
	// don't nest creation modes!
	if (dynamic_cast<ModeCreation*>(parent))
		parent = parent->parent;

	assert(parent);
	menu = parent->menu;
	dialog = NULL;
}

void ModeCreation::OnDrawRecursive(bool multi_view_handled)
{
	Mode::OnDrawRecursive(multi_view_handled);

	ed->DrawStr(MaxX / 2, MaxY - 20, message);
}

void ModeCreation::Abort()
{
	assert(parent);
	ed->SetMode(parent);
}

