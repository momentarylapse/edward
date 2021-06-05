/*
 * ModelNewAnimationDialog.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ModelNewAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelNewAnimationDialog::ModelNewAnimationDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data, int index, AnimationType type):
	hui::Dialog("new_animation_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("new_animation_dialog");
	data = _data;

	set_int("new_animation_index", index);
	set_int("new_animation_type", (int)type - (int)AnimationType::VERTEX);

	event("hui:close", [=]{ on_close(); });
	event("cancel", [=]{ on_close(); });
	event("ok", [=]{ on_ok(); });
}

void ModelNewAnimationDialog::on_close() {
	request_destroy();
}

void ModelNewAnimationDialog::on_ok() {
	int index = get_int("new_animation_index");
	auto type = (AnimationType)(get_int("new_animation_type") + (int)AnimationType::VERTEX);
	if (index < data->move.num)
		if (data->move[index].frame.num > 0) {
			ed->error_box(_("This index is already taken by another animation."));
			return;
		}
	data->addAnimation(index, type);
	request_destroy();
}


