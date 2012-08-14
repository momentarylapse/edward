/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelAnimation.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../Dialog/ModelAnimationDialog.h"

ModeModelAnimation *mode_model_animation = NULL;

ModeModelAnimation::ModeModelAnimation(Mode *_parent, DataModel *_data)
{
	name = "ModelAnimation";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_move");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}


ModeModelAnimation::~ModeModelAnimation()
{
}

void ModeModelAnimation::OnDraw()
{
}



void ModeModelAnimation::OnCommand(const string & id)
{
}



void ModeModelAnimation::OnStart()
{
	// relative to absolute pos
	foreach(data->Bone, b)
		if (b.Parent >= 0)
			b.pos = data->Bone[b.Parent].pos + b.DeltaPos;
		else
			b.pos = b.DeltaPos;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);

	dialog = new ModelAnimationDialog(ed, true, data);
	dialog->Update();
}


void ModeModelAnimation::OnUpdateMenu()
{
}



void ModeModelAnimation::OnEnd()
{
	delete(dialog);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelAnimation::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){

		multi_view->ResetData(data);
		//multi_view->ResetMouseAction();

		// left -> translate
		multi_view->SetMouseAction(0, "ActionModelMVMoveBones", MultiView::ActionMove);

		//mode_model_mesh->ApplyRightMouseFunction(multi_view);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		multi_view->SetData(	MVDSkeletonPoint,
				data->Bone,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
	}
}



void ModeModelAnimation::OnDrawWin(int win, irect dest)
{
	mode_model_skeleton->OnDrawWin(win, dest);
}


