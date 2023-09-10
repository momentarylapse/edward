/*
 * ModelDuplicateAnimationDialog.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ModelDuplicateAnimationDialog.h"
#include "../../../data/model/DataModel.h"
#include "../../../EdwardWindow.h"

ModelDuplicateAnimationDialog::ModelDuplicateAnimationDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data, int index, int _source):
	hui::Dialog("new_animation_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("new_animation_dialog");
	data = _data;
	source = _source;

	set_int("new_animation_index", index);
	hide_control("new_animation_type", true);
	hide_control("new_animation_t_type", true);

	event("hui:close", std::bind(&ModelDuplicateAnimationDialog::on_close, this));
	event("cancel", std::bind(&ModelDuplicateAnimationDialog::on_close, this));
	event("ok", std::bind(&ModelDuplicateAnimationDialog::on_ok, this));
}

void ModelDuplicateAnimationDialog::on_close() {
	request_destroy();
}

void ModelDuplicateAnimationDialog::on_ok() {
	int index = get_int("new_animation_index");
	if (index < data->move.num)
		if (data->move[index].frame.num > 0) {
			data->ed->error_box(_("This index is already taken by another animation."));
			return;
		}
	data->duplicateAnimation(source, index);
	request_destroy();
}


