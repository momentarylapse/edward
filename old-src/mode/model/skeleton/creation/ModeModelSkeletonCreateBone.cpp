/*
 * ModeModelSkeletonCreateBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ModeModelSkeletonCreateBone.h"
#include "../../ModeModel.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"

ModeModelSkeletonCreateBone::ModeModelSkeletonCreateBone(ModeModelSkeleton *_parent) :
	ModeCreation<ModeModelSkeleton, DataModel>("ModelSkeletonCreateBone", _parent)
{
	message = _("Place bone or choose parent bone");
	pos_chosen = false;
	bone_parent = -1;
}


void ModeModelSkeletonCreateBone::on_left_button_down()
{
	if (pos_chosen){
		if (multi_view->hover.index >= 0){
			data->reconnectBone(multi_view->hover.index, bone_parent);
		}else{
			pos = multi_view->get_cursor();
			data->addBone(pos, bone_parent);
		}
		data->bone[bone_parent].is_special = false;
		abort();
	}else{
		if (multi_view->hover.index >= 0){
			bone_parent = multi_view->hover.index;
			data->bone[bone_parent].is_special = true;
			pos_chosen = true;
			message = _("Set bone");
		}else{
			pos = multi_view->get_cursor();
			data->addBone(pos, -1);
			abort();
		}
	}
}

