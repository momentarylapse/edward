/*
 * ObjectPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ObjectPropertiesDialog.h"
#include "../../../Action/World/Object/ActionWorldEditObject.h"
#include <assert.h>

ObjectPropertiesDialog::ObjectPropertiesDialog(hui::Window*_parent, bool _allow_parent, DataWorld *_data, int _index) :
	hui::Dialog("world_object_dialog", 400, 400, _parent, _allow_parent),
	Observer("ObjectPropertiesDialog")
{
	fromResource("world_object_dialog");
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->Objects.num);

	event("cancel", std::bind(&ObjectPropertiesDialog::OnClose, this));
	event("hui:close", std::bind(&ObjectPropertiesDialog::OnClose, this));
	event("apply", std::bind(&ObjectPropertiesDialog::ApplyData, this));
	event("ok", std::bind(&ObjectPropertiesDialog::OnOk, this));

	event("find_object", std::bind(&ObjectPropertiesDialog::OnFindObject, this));

	subscribe(data);

	temp = data->Objects[index];
	LoadData();
}

ObjectPropertiesDialog::~ObjectPropertiesDialog()
{
	unsubscribe(data);
}

void ObjectPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
}



void ObjectPropertiesDialog::LoadData()
{
	setDecimals(3);
	setString("name", temp.Name);
	setString("kind", temp.FileName);
	setFloat("pos_x", temp.pos.x);
	setFloat("pos_y", temp.pos.y);
	setFloat("pos_z", temp.pos.z);
	setFloat("ang_x", temp.Ang.x * 180.0f / pi);
	setFloat("ang_y", temp.Ang.y * 180.0f / pi);
	setFloat("ang_z", temp.Ang.z * 180.0f / pi);
}



void ObjectPropertiesDialog::OnClose()
{
	delete(this);
}



void ObjectPropertiesDialog::OnFindObject()
{
}



void ObjectPropertiesDialog::onUpdate(Observable *o, const string &message)
{
	LoadData();
}



void ObjectPropertiesDialog::ApplyData()
{
	temp.pos.x = getFloat("pos_x");
	temp.pos.y = getFloat("pos_y");
	temp.pos.z = getFloat("pos_z");
	temp.Ang.x = getFloat("ang_x") * pi / 180.0f;
	temp.Ang.y = getFloat("ang_y") * pi / 180.0f;
	temp.Ang.z = getFloat("ang_z") * pi / 180.0f;
	temp.Name = getString("name");

	data->execute(new ActionWorldEditObject(index, temp));
}


