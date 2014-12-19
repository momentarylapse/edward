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
#include "../../../../Action/Model/Skeleton/ActionModelReconnectBone.h"

ModeModelSkeletonCreateBone::ModeModelSkeletonCreateBone(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelSkeletonCreateBone", _parent)
{
	message = _("Knochen setzen oder Wurzel-Knochen w&ahlen");
	pos_chosen = false;
	bone_parent = -1;
}


void ModeModelSkeletonCreateBone::onLeftButtonDown()
{
	if (pos_chosen){
		if (multi_view->hover.index >= 0){
			data->execute(new ActionModelReconnectBone(multi_view->hover.index, bone_parent));
		}else{
			pos = multi_view->getCursor3d();
			data->execute(new ActionModelAddBone(pos, bone_parent));
		}
		data->bone[bone_parent].is_special = false;
		abort();
	}else{
		if (multi_view->hover.index >= 0){
			bone_parent = multi_view->hover.index;
			data->bone[bone_parent].is_special = true;
			pos_chosen = true;
			message = _("Knochen setzen");
		}else{
			pos = multi_view->getCursor3d();
			data->execute(new ActionModelAddBone(pos, -1));
			abort();
		}
	}
}

