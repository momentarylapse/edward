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

void ModeWorldCreateObject::on_start()
{
	filename = LastObjectFilename;
	if (filename.num == 0)
		onFindObject();

	dialog = new hui::Panel();
	dialog->from_resource("world_new_object_dialog");
	dialog->set_string("kind", filename);
	dialog->enable("name", false);
	dialog->event("find_object", std::bind(&ModeWorldCreateObject::onFindObject, this));
	ed->set_side_panel(dialog);

	if (filename.num > 0)
		message = _("neues Objekt setzen");

	ed->activate("");
}


void ModeWorldCreateObject::on_end()
{
	ed->set_side_panel(nullptr);
}

void ModeWorldCreateObject::onFindObject()
{
	if (ed->file_dialog(FD_MODEL, false, true)){
		filename = ed->dialog_file_no_ending;
		LastObjectFilename = filename;
		message = _("neues Objekt setzen");
	}
}


void ModeWorldCreateObject::on_left_button_down()
{
	if (filename.num > 0){
		data->AddObject(filename, multi_view->get_cursor());
		//Abort();
	}
}
