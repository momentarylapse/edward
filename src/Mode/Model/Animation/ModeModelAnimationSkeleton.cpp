/*
 * ModeModelAnimationSkeleton.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationSkeleton.h"
#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "../Mesh/ModeModelMeshPolygon.h"
#include "../Skeleton/ModeModelSkeleton.h"

ModeModelAnimationSkeleton *mode_model_animation_skeleton = NULL;

ModeModelAnimationSkeleton::ModeModelAnimationSkeleton(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationSkeleton", _parent, ed->multi_view_3d, "menu_move")
{
}

ModeModelAnimationSkeleton::~ModeModelAnimationSkeleton()
{
}

void ModeModelAnimationSkeleton::OnStart()
{
	multi_view->ResetData(NULL);

	// left -> translate
	//multi_view->SetMouseAction(0, "ActionModelAnimationMoveBones", MultiView::ActionMove);
	multi_view->SetMouseAction(1, "ActionModelAnimationRotateBones", MultiView::ActionRotate);
	multi_view->SetMouseAction(2, "ActionModelAnimationRotateBones", MultiView::ActionRotate2d);
	multi_view->MVRectable = true;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);
}

void ModeModelAnimationSkeleton::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	multi_view->ResetData(NULL);
}

void ModeModelAnimationSkeleton::OnCommand(const string& id)
{
}

void ModeModelAnimationSkeleton::OnUpdate(Observable* o)
{
	if (o->GetName() == "Data"){

		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->SetData(	MVDSkeletonPoint,
				data->Bone,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
	}
}

void ModeModelAnimationSkeleton::OnUpdateMenu()
{
}

void ModeModelAnimationSkeleton::OnDrawWin(MultiViewWindow *win)
{
	mode_model_skeleton->OnDrawWin(win);
}


