/*
 * ModeModelSkeletonCreateBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ModeModelSkeletonCreateBone.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../Action/Model/Skeleton/ActionModelAddBone.h"

ModeModelSkeletonCreateBone::ModeModelSkeletonCreateBone(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelSkeletonCreateBone", _parent)
{
	message = _("Knochen setzen oder Wurzel-Knochen w&ahlen");
	pos_chosen = false;
	bone_parent = -1;
}

ModeModelSkeletonCreateBone::~ModeModelSkeletonCreateBone()
{
}

void ModeModelSkeletonCreateBone::onDrawWin(MultiView::Window *win)
{
}



void ModeModelSkeletonCreateBone::onLeftButtonDown()
{
	if (pos_chosen){
		pos = multi_view->GetCursor3d();
		data->execute(new ActionModelAddBone(pos, bone_parent));
		data->Bone[bone_parent].is_special = false;
		abort();
	}else{
		if (multi_view->hover.index >= 0){
			bone_parent = multi_view->hover.index;
			data->Bone[bone_parent].is_special = true;
			pos_chosen = true;
			message = _("Knochen setzen");
		}else{
			pos = multi_view->GetCursor3d();
			data->execute(new ActionModelAddBone(pos, -1));
			abort();
		}
	}
}



void ModeModelSkeletonCreateBone::onMouseMove()
{
}


