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

ModeModelSkeletonCreateBone::ModeModelSkeletonCreateBone(Mode *_parent, DataModel *_data)
{
	name = "ModelSkeletonCreateBone";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("Knochen setzen oder Wurzel-Knochen waehlen");
	pos_chosen = false;
	bone_parent = -1;
}

ModeModelSkeletonCreateBone::~ModeModelSkeletonCreateBone()
{
}

void ModeModelSkeletonCreateBone::PostDrawWin(int win, irect dest)
{
}



void ModeModelSkeletonCreateBone::OnLeftButtonDown()
{
	if (pos_chosen){
		pos = multi_view->GetCursor3d();
		data->Execute(new ActionModelAddBone(pos, bone_parent));
		data->Bone[bone_parent].is_special = false;
		ed->SetCreationMode(NULL);
	}else{
		if (multi_view->Selected >= 0){
			bone_parent = multi_view->Selected;
			data->Bone[bone_parent].is_special = true;
			pos_chosen = true;
			message = _("Knochen setzen");
		}else{
			pos = multi_view->GetCursor3d();
			data->Execute(new ActionModelAddBone(pos, -1));
			ed->SetCreationMode(NULL);
		}
	}
}



void ModeModelSkeletonCreateBone::OnMouseMove()
{
}


