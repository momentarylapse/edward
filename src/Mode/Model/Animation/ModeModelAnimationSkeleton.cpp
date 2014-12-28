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
	Mode<DataModel>("ModelAnimationSkeleton", _parent, ed->multi_view_3d, "menu_move"),
	current_move(((ModeModelAnimation*)parent)->current_move),
	current_frame(((ModeModelAnimation*)parent)->current_frame)
{
	select_recursive = true;
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
	t->addItemCheckable(_("Rekursiv bearbeiten"), "hui:apply", "select-recursive");
	t->enable(true);
	t->configure(false,true);

	foreachi(ModelBone &b, data->bone, i)
		mode_model_animation->bone[i].is_selected = b.is_selected;

	multi_view->clearData(data);

	chooseMouseFunction(MultiView::ACTION_ROTATE);

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
	if (id == "select-recursive"){
		select_recursive = !select_recursive;
		updateSelection();
		ed->updateMenu();
	}
	if (id == "copy")
		copy();
	if (id == "paste")
		paste();
}

void ModeModelAnimationSkeleton::chooseMouseFunction(int f)
{
	mouse_action = f;

	// mouse action
	multi_view->setMouseAction("ActionModelAnimationTransformBones", mouse_action, false);
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
		updateSelection();
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

	ed->check("select-recursive", select_recursive);
}

void ModeModelAnimationSkeleton::updateSelection()
{
	foreachi(ModelBone &b, data->bone, i)
		b.is_selected = mode_model_animation->bone[i].is_selected;

	// also select children?
	if (select_recursive){
		// this works thanks to lucky circumstances... might break in the future
		foreachi(ModelBone &b, data->bone, i)
			if (b.is_selected){
				foreach(ModelBone &bc, data->bone)
					if (bc.parent == i)
						bc.is_selected = true;
			}
	}
}

void ModeModelAnimationSkeleton::copy()
{
	int n = data->getNumSelectedBones();
	if (n == 0){
		ed->setMessage(_("nichts markiert"));
		return;
	}else if (n == 1){
		temp.clear();
		ModelFrame f;
		foreachi(ModelBone &b, data->bone, i)
			if (b.is_selected){
				f.skel_ang.add(mode_model_animation->cur_move()->frame[current_frame].skel_ang[i]);
				f.skel_dpos.add(mode_model_animation->cur_move()->frame[current_frame].skel_dpos[i]);
			}
		temp.add(f);
		ed->setMessage(format(_("Animation von einzelnem Knochen kopiert"), n));
	}else{
		temp.clear();
		temp.add(mode_model_animation->cur_move()->frame[current_frame]);
		ed->setMessage(format(_("Animation von allen &d Knochen kopiert"), data->bone.num));
	}
}

void ModeModelAnimationSkeleton::paste()
{
	if (temp.num == 0){
		ed->setMessage(_("Zwischenablage leer"));
		return;
	}
	data->beginActionGroup("paste-animation");
	int n = data->getNumSelectedBones();
	int nt = temp[0].skel_ang.num;
	if (nt == 1){
		foreachi(ModelBone &b, data->bone, i)
			if (b.is_selected)
				data->animationSetBone(current_move, current_frame, i, temp[0].skel_dpos[0], temp[0].skel_ang[0]);
		if (n == 1)
			ed->setMessage(_("Animation eingef&ugt - einzelner Knochen"));
		else
			ed->setMessage(_("Animation eingef&ugt - einzelner Knochen auf mehrere"));
	}else{
		foreachi(ModelBone &b, data->bone, i)
			if (b.is_selected)
				data->animationSetBone(current_move, current_frame, i, temp[0].skel_dpos[i], temp[0].skel_ang[i]);
		ed->setMessage(format(_("Animation eingef&ugt -  auf &d Knochen"), n));
	}
	data->endActionGroup();
}
