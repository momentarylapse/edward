/*
 * ActionCameraAddPoint.h
 *
 *  Created on: 03.10.2012
 *      Author: michi
 */

#ifndef ACTIONCAMERAADDPOINT_H_
#define ACTIONCAMERAADDPOINT_H_

#include "../../Action.h"
#include "../../../lib/math/math.h"
class DataCamera;

class ActionCameraAddPoint: public Action
{
public:
	ActionCameraAddPoint(DataCamera *d, const vector &_pos, const vector &_vel, const vector &_ang, float _dt);
	virtual ~ActionCameraAddPoint(){}
	string name(){	return "CameraAddPoint";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	vector pos, ang, vel;
	float dt;
};

#endif /* ACTIONCAMERAADDPOINT_H_ */
