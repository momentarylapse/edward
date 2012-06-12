/*
 * ModeWorldCreateObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateObject.h"
#include "../../../Action/World/ActionWorldAddObject.h"
#include "../../../Edward.h"

ModeWorldCreateObject::ModeWorldCreateObject(Mode *_parent, DataWorld *_data)
{
	name = "WorldCreateObject";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("neues Objekt setzen");
}

ModeWorldCreateObject::~ModeWorldCreateObject()
{
}


void ModeWorldCreateObject::OnLeftButtonDown()
{
	if (ed->FileDialog(FDModel, false, true)){
		filename = ed->DialogFileNoEnding;
		data->Execute(new ActionWorldAddObject(filename, multi_view->GetCursor3d()));
	}
	//ed->SetCreationMode(NULL);
}
