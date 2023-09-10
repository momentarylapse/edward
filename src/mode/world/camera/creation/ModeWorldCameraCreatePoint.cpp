/*
 * ModeWorldCameraCreatePoint.cpp
 *
 *  Created on: 07.10.2012
 *      Author: michi
 */

#include "ModeWorldCameraCreatePoint.h"
#include "../../../../data/world/DataCamera.h"
#include "../../../../action/world/camera/ActionCameraAddPoint.h"
#include "../../../../multiview/MultiView.h"

ModeWorldCameraCreatePoint::ModeWorldCameraCreatePoint(ModeWorldCamera *_parent) :
	ModeCreation<ModeWorldCamera, DataCamera>("WorldCameraCreatePoint", _parent)
{
	message = _("Add point");
}

void ModeWorldCameraCreatePoint::on_left_button_down() {
	vec3 pos = multi_view->get_cursor();
	data->execute(new ActionCameraAddPoint(data, pos, v_0, v_0, 1));
	abort();
}

