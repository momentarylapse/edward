/*
 * ActionCameraAddPoint.cpp
 *
 *  Created on: 03.10.2012
 *      Author: michi
 */

#include "ActionCameraAddPoint.h"
#include "../../../Data/World/DataCamera.h"

ActionCameraAddPoint::ActionCameraAddPoint(DataCamera *d, const vector &_pos, const vector &_vel, const vector &_ang, float _dt)
{
	pos = _pos;
	vel = _vel;
	ang = _ang;
	dt = _dt;
}

void* ActionCameraAddPoint::execute(Data* d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);

	WorldCamPoint p;
	p.view_stage = 0;
	p.is_selected = false;
	p.is_special = false;
	p.pos = pos;
	p.Ang = ang;
	p.Vel = vel;
	p.Duration = dt;
	p.Type = CPKCamFlight;
	w->Point.add(p);
	return NULL;
}

void ActionCameraAddPoint::undo(Data* d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);

	w->Point.pop();
}


