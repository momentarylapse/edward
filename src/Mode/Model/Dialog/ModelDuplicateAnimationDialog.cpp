/*
 * ModelDuplicateAnimationDialog.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ModelDuplicateAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelDuplicateAnimationDialog::ModelDuplicateAnimationDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data, int index, int _source):
	hui::Dialog("new_animation_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("new_animation_dialog");
	data = _data;
	source = _source;

	set_int("new_animation_index", index);
	hide_control("new_animation_type", true);
	hide_control("new_animation_t_type", true);

	event("hui:close", std::bind(&ModelDuplicateAnimationDialog::onClose, this));
	event("cancel", std::bind(&ModelDuplicateAnimationDialog::onClose, this));
	event("ok", std::bind(&ModelDuplicateAnimationDialog::onOk, this));
}

ModelDuplicateAnimationDialog::~ModelDuplicateAnimationDialog()
{
}

void ModelDuplicateAnimationDialog::onClose()
{
	destroy();
}

void ModelDuplicateAnimationDialog::onOk()
{
	int index = get_int("new_animation_index");
	if (index < data->move.num)
		if (data->move[index].frame.num > 0){
			ed->error_box(_("Es existiert bereits eine Animation mit diesem Index. Bitte einen anderen w&ahlen."));
			return;
		}
	data->duplicateAnimation(source, index);
	destroy();
}


