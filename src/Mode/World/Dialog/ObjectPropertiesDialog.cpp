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
	from_resource("world_object_dialog");
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
	destroy();
}



void ObjectPropertiesDialog::LoadData()
{
	set_decimals(3);
	set_string("name", temp.Name);
	set_string("kind", temp.FileName);
	set_float("pos_x", temp.pos.x);
	set_float("pos_y", temp.pos.y);
	set_float("pos_z", temp.pos.z);
	set_float("ang_x", temp.Ang.x * 180.0f / pi);
	set_float("ang_y", temp.Ang.y * 180.0f / pi);
	set_float("ang_z", temp.Ang.z * 180.0f / pi);
}



void ObjectPropertiesDialog::OnClose()
{
	destroy();
}



void ObjectPropertiesDialog::OnFindObject()
{
}



void ObjectPropertiesDialog::on_update(Observable *o, const string &message)
{
	LoadData();
}



void ObjectPropertiesDialog::ApplyData()
{
	temp.pos.x = get_float("pos_x");
	temp.pos.y = get_float("pos_y");
	temp.pos.z = get_float("pos_z");
	temp.Ang.x = get_float("ang_x") * pi / 180.0f;
	temp.Ang.y = get_float("ang_y") * pi / 180.0f;
	temp.Ang.z = get_float("ang_z") * pi / 180.0f;
	temp.Name = get_string("name");

	data->execute(new ActionWorldEditObject(index, temp));
}


