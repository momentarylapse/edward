/*
 * ActionCameraAddPoint.cpp
 *
 *  Created on: 03.10.2012
 *      Author: michi
 */

#include "ActionCameraAddPoint.h"
#include "../../../data/world/DataCamera.h"

ActionCameraAddPoint::ActionCameraAddPoint(DataCamera *d, const vec3 &_pos, const vec3 &_vel, const vec3 &_ang, float _dt)
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


