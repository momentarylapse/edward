/*
 * ModeCreation.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeCreation.h"
#include "../Edward.h"
#include <assert.h>

ModeCreation::ModeCreation(Mode *_parent)
{
	parent = _parent;

	// don't nest creation modes!
	if (dynamic_cast<ModeCreation*>(parent))
		parent = parent->parent;

	assert(parent);
	menu = parent->menu;
	multi_view = parent->multi_view;
	dialog = NULL;
}

void ModeCreation::OnDrawRecursive()
{
	if (parent)
		parent->OnDrawRecursive();
	OnDraw();

	ed->DrawStr(MaxX / 2, MaxY - 20, message);
}

void ModeCreation::Abort()
{
	assert(parent);
	ed->SetMode(parent);
}

