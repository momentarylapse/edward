/*
 * ModelDuplicateAnimationDialog.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ModelDuplicateAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelDuplicateAnimationDialog::ModelDuplicateAnimationDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data, int index, int _source):
	HuiWindow("new_animation_dialog", _parent, _allow_parent)
{
	data = _data;
	source = _source;

	setInt("new_animation_index", index);
	hideControl("new_animation_type", true);
	hideControl("new_animation_t_type", true);

	event("hui:close", this, &ModelDuplicateAnimationDialog::onClose);
	event("cancel", this, &ModelDuplicateAnimationDialog::onClose);
	event("ok", this, &ModelDuplicateAnimationDialog::onOk);
}

ModelDuplicateAnimationDialog::~ModelDuplicateAnimationDialog()
{
}

void ModelDuplicateAnimationDialog::onClose()
{
	delete(this);
}

void ModelDuplicateAnimationDialog::onOk()
{
	int index = getInt("new_animation_index");
	if (index < data->move.num)
		if (data->move[index].frame.num > 0){
			ed->errorBox(_("Es existiert bereits eine Animation mit diesem Index. Bitte einen anderen w&ahlen."));
			return;
		}
	data->duplicateAnimation(source, index);
	delete(this);
}


