/*
 * ModeModelAnimationSkeleton.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationSkeleton.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../Mesh/ModeModelMeshPolygon.h"
#include "../Skeleton/ModeModelSkeleton.h"

ModeModelAnimationSkeleton *mode_model_animation_skeleton = NULL;

void DrawBone(const vector &r, const vector &d, const color &c, MultiView::Window *win);

ModeModelAnimationSkeleton::ModeModelAnimationSkeleton(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationSkeleton", _parent, ed->multi_view_3d, "menu_move")
{
}

ModeModelAnimationSkeleton::~ModeModelAnimationSkeleton()
{
}

void ModeModelAnimationSkeleton::onStart()
{
	multi_view->ClearData(NULL);

	// left -> translate
	multi_view->SetMouseAction("ActionModelAnimationMoveBones", MultiView::ActionSelectAndMove);
	//multi_view->SetMouseAction(0, "ActionModelAnimationMoveBones", MultiView::ActionMove);
//	multi_view->SetMouseAction(1, "ActionModelAnimationRotateBones", MultiView::ActionRotate);
//	multi_view->SetMouseAction(2, "ActionModelAnimationRotateBones", MultiView::ActionRotate2d);
	multi_view->allow_rect = true;

	subscribe(data);
	subscribe(multi_view, "SelectionChange");
	onUpdate(data);
}

void ModeModelAnimationSkeleton::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
	multi_view->ClearData(NULL);
}

void ModeModelAnimationSkeleton::onCommand(const string& id)
{
}

void ModeModelAnimationSkeleton::onUpdate(Observable* o)
{
	if (o->getName() == "Data"){

		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->AddData(	MVDSkeletonPoint,
				data->Bone,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect);
	}else if (o->getName() == "MultiView"){
	}
}

void ModeModelAnimationSkeleton::onUpdateMenu()
{
}

void ModeModelAnimationSkeleton::onDrawWin(MultiView::Window *win)
{
	mode_model_skeleton->onDrawWin(win);
#if 0
	NixSetZ(false, false);
	NixEnableLighting(false);
	NixSetWire(false);

	foreach(ModelBone &b, data->Bone){
		/*if (b.view_stage<=ViewStage)
			continue;*/
//		if (b.is_selected)
//			DrawCoordBasis(&b);
		int r = b.Parent;
		if (r < 0)
			continue;
		color c = data->Bone[r].is_selected ? Red : Blue;
		if (multi_view->MouseOver == r)
			c = ColorInterpolate(c, White, 0.3f);
		DrawBone(data->Bone[r].pos, b.pos, c, win);
	}
	NixSetZ(true, true);
#endif
}


