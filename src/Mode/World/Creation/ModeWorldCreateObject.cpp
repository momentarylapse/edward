/*
 * ModeWorldCreateObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateObject.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Edward.h"

static string LastObjectFilename;

ModeWorldCreateObject::ModeWorldCreateObject(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateObject", _parent)
{
	message = _("neues Objekt ausw&ahlen");
}

ModeWorldCreateObject::~ModeWorldCreateObject()
{
}

void ModeWorldCreateObject::OnStart()
{
	filename = LastObjectFilename;
	if (filename.num == 0)
		OnFindObject();

	dialog = HuiCreateResourceDialog("world_new_object_dialog",ed);
	dialog->SetString("kind", filename);
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Enable("name", false);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);
	dialog->EventM("find_object", this, &ModeWorldCreateObject::OnFindObject);

	if (filename.num > 0)
		message = _("neues Objekt setzen");

	ed->Activate();
}


void ModeWorldCreateObject::OnEnd()
{
	delete(dialog);
}

void ModeWorldCreateObject::OnFindObject()
{
	if (ed->FileDialog(FDModel, false, true)){
		filename = ed->DialogFileNoEnding;
		LastObjectFilename = filename;
		message = _("neues Objekt setzen");
	}
}


void ModeWorldCreateObject::OnLeftButtonDown()
{
	if (filename.num > 0){
		data->AddObject(filename, multi_view->GetCursor3d());
		//Abort();
	}
}
