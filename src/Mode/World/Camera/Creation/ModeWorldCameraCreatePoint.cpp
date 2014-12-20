/*
 * ModeWorldCameraCreatePoint.cpp
 *
 *  Created on: 07.10.2012
 *      Author: michi
 */

#include "ModeWorldCameraCreatePoint.h"
#include "../../../../Data/World/DataCamera.h"
#include "../../../../Action/World/Camera/ActionCameraAddPoint.h"
#include "../../../../MultiView/MultiView.h"

ModeWorldCameraCreatePoint::ModeWorldCameraCreatePoint(ModeBase *_parent) :
	ModeCreation<DataCamera>("WorldCameraCreatePoint", _parent)
{
	message = _("Punkt setzen");
}

void ModeWorldCameraCreatePoint::onLeftButtonDown()
{
	vector pos = multi_view->getCursor3d();
	data->execute(new ActionCameraAddPoint(data, pos, v_0, v_0, 1));
	abort();
}

