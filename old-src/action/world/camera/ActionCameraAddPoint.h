/*
 * ActionCameraAddPoint.h
 *
 *  Created on: 03.10.2012
 *      Author: michi
 */

#ifndef ACTIONCAMERAADDPOINT_H_
#define ACTIONCAMERAADDPOINT_H_

#include "../../../lib/math/vec3.h"
#include "../../Action.h"
class DataCamera;

class ActionCameraAddPoint: public Action {
public:
	ActionCameraAddPoint(DataCamera *d, const vec3 &_pos, const vec3 &_vel, const vec3 &_ang, float _dt);
	string name(){	return "CameraAddPoint";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	vec3 pos, ang, vel;
	float dt;
};

#endif /* ACTIONCAMERAADDPOINT_H_ */
