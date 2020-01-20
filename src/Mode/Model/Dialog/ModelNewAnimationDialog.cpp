/*
 * ModelNewAnimationDialog.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ModelNewAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelNewAnimationDialog::ModelNewAnimationDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data, int index, int type):
	hui::Dialog("new_animation_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("new_animation_dialog");
	data = _data;

	set_int("new_animation_index", index);
	set_int("new_animation_type", type - MOVE_TYPE_VERTEX);

	event("hui:close", std::bind(&ModelNewAnimationDialog::onClose, this));
	event("cancel", std::bind(&ModelNewAnimationDialog::onClose, this));
	event("ok", std::bind(&ModelNewAnimationDialog::onOk, this));
}

ModelNewAnimationDialog::~ModelNewAnimationDialog()
{
}

void ModelNewAnimationDialog::onClose()
{
	destroy();
}

void ModelNewAnimationDialog::onOk()
{
	int index = get_int("new_animation_index");
	int type = get_int("new_animation_type") + MOVE_TYPE_VERTEX;
	if (index < data->move.num)
		if (data->move[index].frame.num > 0){
			ed->error_box(_("This index is already taken by another animation."));
			return;
		}
	data->addAnimation(index, type);
	destroy();
}


