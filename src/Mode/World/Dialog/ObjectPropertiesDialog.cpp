/*
 * ObjectPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ObjectPropertiesDialog.h"
#include "../../../Action/World/ActionWorldEditObject.h"
#include <assert.h>

ObjectPropertiesDialog::ObjectPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->Objects.num);

	// dialog
	FromResource("world_object_dialog");

	EventM("cancel", this, &ObjectPropertiesDialog::OnClose);
	EventM("hui:close", this, &ObjectPropertiesDialog::OnClose);
	EventM("apply", this, &ObjectPropertiesDialog::ApplyData);
	EventM("ok", this, &ObjectPropertiesDialog::OnOk);

	EventM("find_object", this, &ObjectPropertiesDialog::OnFindObject);

	Subscribe(data);

	temp = data->Objects[index];
	LoadData();
}

ObjectPropertiesDialog::~ObjectPropertiesDialog()
{
	Unsubscribe(data);
}

void ObjectPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
}



void ObjectPropertiesDialog::LoadData()
{
	SetDecimals(3);
	SetString("name", temp.Name);
	SetString("kind", temp.FileName);
	SetFloat("pos_x", temp.pos.x);
	SetFloat("pos_y", temp.pos.y);
	SetFloat("pos_z", temp.pos.z);
	SetFloat("ang_x", temp.Ang.x * 180.0f / pi);
	SetFloat("ang_y", temp.Ang.y * 180.0f / pi);
	SetFloat("ang_z", temp.Ang.z * 180.0f / pi);
}



void ObjectPropertiesDialog::OnClose()
{
	delete(this);
}



void ObjectPropertiesDialog::OnFindObject()
{
}



void ObjectPropertiesDialog::OnUpdate(Observable *o)
{
	LoadData();
}



void ObjectPropertiesDialog::ApplyData()
{
	temp.pos.x = GetFloat("pos_x");
	temp.pos.y = GetFloat("pos_y");
	temp.pos.z = GetFloat("pos_z");
	temp.Ang.x = GetFloat("ang_x") * pi / 180.0f;
	temp.Ang.y = GetFloat("ang_y") * pi / 180.0f;
	temp.Ang.z = GetFloat("ang_z") * pi / 180.0f;
	temp.Name = GetString("name");

	data->Execute(new ActionWorldEditObject(index, temp));
}


