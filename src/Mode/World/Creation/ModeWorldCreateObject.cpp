/*
 * ModeWorldCreateObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateObject.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"

static string LastObjectFilename;

ModeWorldCreateObject::ModeWorldCreateObject(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateObject", _parent)
{
	message = _("neues Objekt ausw&ahlen");
}

ModeWorldCreateObject::~ModeWorldCreateObject()
{
}

void ModeWorldCreateObject::onStart()
{
	filename = LastObjectFilename;
	if (filename.num == 0)
		OnFindObject();

	dialog = HuiCreateResourceDialog("world_new_object_dialog",ed);
	dialog->setString("kind", filename);
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->enable("name", false);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);
	dialog->event("find_object", this, &ModeWorldCreateObject::OnFindObject);

	if (filename.num > 0)
		message = _("neues Objekt setzen");

	ed->activate("");
}


void ModeWorldCreateObject::onEnd()
{
	delete(dialog);
}

void ModeWorldCreateObject::OnFindObject()
{
	if (ed->fileDialog(FDModel, false, true)){
		filename = ed->DialogFileNoEnding;
		LastObjectFilename = filename;
		message = _("neues Objekt setzen");
	}
}


void ModeWorldCreateObject::onLeftButtonDown()
{
	if (filename.num > 0){
		data->AddObject(filename, multi_view->GetCursor3d());
		//Abort();
	}
}
