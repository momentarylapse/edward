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

	multi_view->clearData(data);
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

	// mouse action
	if (mouse_action != MultiView::ACTION_SELECT){
		multi_view->setMouseAction("ActionModelAnimationTransformBones", mouse_action);
	}else{
		multi_view->setMouseAction("", MultiView::ACTION_SELECT);
	}
}

void ModeModelAnimationSkeleton::onUpdate(Observable* o, const string &message)
{
	if (o == data){

		multi_view->clearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->addData(	MVD_SKELETON_BONE,
				mode_model_animation->bone,
				NULL,
				MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT);
	}else if (o == multi_view){
		foreachi(ModelBone &b, data->bone, i)
			b.is_selected = mode_model_animation->bone[i].is_selected;
	}
}

void ModeModelAnimationSkeleton::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh->drawPolygons(win, mode_model_animation->vertex);
	mode_model_skeleton->drawSkeleton(win, mode_model_animation->bone);
}



void ModeModelAnimationSkeleton::onUpdateMenu()
{
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
}
