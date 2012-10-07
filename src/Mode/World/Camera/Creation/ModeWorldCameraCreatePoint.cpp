/*
 * ModeWorldCameraCreatePoint.cpp
 *
 *  Created on: 07.10.2012
 *      Author: michi
 */

#include "ModeWorldCameraCreatePoint.h"
#include "../../../../Data/World/DataCamera.h"
#include "../../../../Action/World/Camera/ActionCameraAddPoint.h"

ModeWorldCameraCreatePoint::ModeWorldCameraCreatePoint(Mode *_parent) :
	ModeCreation("WorldCameraCreatePoint", _parent)
{
	data = (DataCamera*)_parent->GetData();

	message = _("Punkt setzen");
}

ModeWorldCameraCreatePoint::~ModeWorldCameraCreatePoint()
{
}

void ModeWorldCameraCreatePoint::OnLeftButtonDown()
{
	vector pos = multi_view->GetCursor3d();
	data->Execute(new ActionCameraAddPoint(data, pos, v_0, v_0, 1));
	Abort();
}

