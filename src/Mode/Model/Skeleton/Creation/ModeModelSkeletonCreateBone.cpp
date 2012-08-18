/*
 * ModeModelSkeletonCreateBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ModeModelSkeletonCreateBone.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Skeleton/ActionModelAddBone.h"

ModeModelSkeletonCreateBone::ModeModelSkeletonCreateBone(Mode *_parent) :
	ModeCreation("ModelSkeletonCreateBone", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Knochen setzen oder Wurzel-Knochen waehlen");
	pos_chosen = false;
	bone_parent = -1;
}

ModeModelSkeletonCreateBone::~ModeModelSkeletonCreateBone()
{
}

void ModeModelSkeletonCreateBone::OnDrawWin(int win, irect dest)
{
}



void ModeModelSkeletonCreateBone::OnLeftButtonDown()
{
	if (pos_chosen){
		pos = multi_view->GetCursor3d();
		data->Execute(new ActionModelAddBone(pos, bone_parent));
		data->Bone[bone_parent].is_special = false;
		Abort();
	}else{
		if (multi_view->Selected >= 0){
			bone_parent = multi_view->Selected;
			data->Bone[bone_parent].is_special = true;
			pos_chosen = true;
			message = _("Knochen setzen");
		}else{
			pos = multi_view->GetCursor3d();
			data->Execute(new ActionModelAddBone(pos, -1));
			Abort();
		}
	}
}



void ModeModelSkeletonCreateBone::OnMouseMove()
{
}


