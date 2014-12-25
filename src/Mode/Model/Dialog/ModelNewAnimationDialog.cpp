/*
 * ModelNewAnimationDialog.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ModelNewAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelNewAnimationDialog::ModelNewAnimationDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data, int index, int type):
	HuiWindow("new_animation_dialog", _parent, _allow_parent)
{
	data = _data;

	setInt("new_animation_index", index);
	setInt("new_animation_type", type - MOVE_TYPE_VERTEX);

	event("hui:close", this, &ModelNewAnimationDialog::onClose);
	event("cancel", this, &ModelNewAnimationDialog::onClose);
	event("ok", this, &ModelNewAnimationDialog::onOk);
}

ModelNewAnimationDialog::~ModelNewAnimationDialog()
{
}

void ModelNewAnimationDialog::onClose()
{
	delete(this);
}

void ModelNewAnimationDialog::onOk()
{
	int index = getInt("new_animation_index");
	int type = getInt("new_animation_type") + MOVE_TYPE_VERTEX;
	if (index < data->move.num)
		if (data->move[index].frame.num > 0){
			ed->errorBox(_("Es existiert bereits eine Animation mit diesem Index. Bitte einen anderen w&ahlen."));
			return;
		}
	data->addAnimation(index, type);
	delete(this);
}


