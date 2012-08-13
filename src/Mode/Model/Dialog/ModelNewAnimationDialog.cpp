/*
 * ModelNewAnimationDialog.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ModelNewAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"

ModelNewAnimationDialog::ModelNewAnimationDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data, int index):
	CHuiWindow("dummy", -1, -1, 320, 450, _parent, _allow_parent, HuiWinModeControls , true)
{
	// dialog
	FromResource("new_animation_dialog");

	data = _data;

	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelNewAnimationDialog::OnClose);
	EventM("hui:cancel", this, (void(HuiEventHandler::*)())&ModelNewAnimationDialog::OnClose);
	EventM("hui:ok", this, (void(HuiEventHandler::*)())&ModelNewAnimationDialog::OnOk);
}

ModelNewAnimationDialog::~ModelNewAnimationDialog()
{
}

void ModelNewAnimationDialog::OnClose()
{
}

void ModelNewAnimationDialog::OnOk()
{
	/*NewMoveIndex=dlg->GetInt(HMM_NEW_ANIMATION_INDEX);
	NewMoveType=dlg->GetInt(HMM_NEW_ANIMATION_TYPE)+MoveTypeVertex;
	if (mmodel->Move[NewMoveIndex].Frame.num>0){
		CModeAll::ErrorBox(_("Es existiert bereits eine Animation mit diesem Index. Bitte einen anderen w&ahlen."));
		return;
	}*/
}


