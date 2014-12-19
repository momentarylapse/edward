/*
 * ModeModelAnimationSkeleton.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationSkeleton.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../Animation/ModeModelAnimation.h"
#include "../Mesh/ModeModelMesh.h"

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
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->addSeparator();
	t->addItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->addItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->addItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->enable(true);
	t->configure(false,true);

	foreachi(ModelBone &b, data->bone, i)
		mode_model_animation->bone[i].is_selected = b.is_selected;

	multi_view->ClearData(NULL);
	multi_view->allow_rect = true;

	chooseMouseFunction(MultiView::ACTION_SELECT);

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	onUpdate(data, "");
}

void ModeModelAnimationSkeleton::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
	multi_view->ClearData(NULL);
}

void ModeModelAnimationSkeleton::onCommand(const string& id)
{
	if (id == "select")
		chooseMouseFunction(MultiView::ACTION_SELECT);
	if (id == "translate")
		chooseMouseFunction(MultiView::ACTION_MOVE);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ACTION_ROTATE);
}

void ModeModelAnimationSkeleton::chooseMouseFunction(int f)
{
	mouse_action = f;
	ed->updateMenu();

	// mouse action
	if (mouse_action != MultiView::ACTION_SELECT){
		multi_view->SetMouseAction("ActionModelAnimationTransformBones", mouse_action);
	}else{
		multi_view->SetMouseAction("", MultiView::ACTION_SELECT);
	}
}

void ModeModelAnimationSkeleton::onUpdate(Observable* o, const string &message)
{
	if (o == data){

		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->AddData(	MVD_SKELETON_BONE,
				mode_model_animation->bone,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect);
	}else if (o == multi_view){
		foreachi(ModelBone &b, data->bone, i)
			b.is_selected = mode_model_animation->bone[i].is_selected;
	}
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



void ModeModelAnimationSkeleton::onUpdateMenu()
{
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
}
