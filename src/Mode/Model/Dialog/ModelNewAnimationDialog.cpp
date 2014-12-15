/*
 * ModelNewAnimationDialog.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ModelNewAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelNewAnimationDialog::ModelNewAnimationDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data, int index):
	HuiWindow("new_animation_dialog", _parent, _allow_parent)
{
	data = _data;

	setInt("new_animation_index", index);

	event("hui:close", this, &ModelNewAnimationDialog::OnClose);
	event("cancel", this, &ModelNewAnimationDialog::OnClose);
	event("ok", this, &ModelNewAnimationDialog::OnOk);
}

ModelNewAnimationDialog::~ModelNewAnimationDialog()
{
}

void ModelNewAnimationDialog::OnClose()
{
	delete(this);
}

void ModelNewAnimationDialog::OnOk()
{
	int index = getInt("new_animation_index");
	int type = getInt("new_animation_type") + MoveTypeVertex;
	if (index < data->move.num)
		if (data->move[index].frame.num > 0){
			ed->errorBox(_("Es existiert bereits eine Animation mit diesem Index. Bitte einen anderen w&ahlen."));
			return;
		}
	data->AddAnimation(index, type);
	delete(this);
}


