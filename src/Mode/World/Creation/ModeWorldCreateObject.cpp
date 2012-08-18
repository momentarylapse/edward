/*
 * ModeWorldCreateObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateObject.h"
#include "../../../Edward.h"

ModeWorldCreateObject::ModeWorldCreateObject(Mode *_parent) :
	ModeCreation("WorldCreateObject", _parent)
{
	data = (DataWorld*)_parent->GetData();

	message = _("neues Objekt setzen");
}

ModeWorldCreateObject::~ModeWorldCreateObject()
{
}


void ModeWorldCreateObject::OnLeftButtonDown()
{
	if (ed->FileDialog(FDModel, false, true)){
		filename = ed->DialogFileNoEnding;
		data->AddObject(filename, multi_view->GetCursor3d());
	}
	//ed->SetCreationMode(NULL);
}
