/*
 * ActionCameraMoveSelection.h
 *
 *  Created on: 02.10.2012
 *      Author: michi
 */

#ifndef ACTIONCAMERAMOVESELECTION_H_
#define ACTIONCAMERAMOVESELECTION_H_

#include "../../ActionMultiView.h"
class DataCamera;
class vec3;

class ActionCameraMoveSelection: public ActionMultiView
{
public:
	ActionCameraMoveSelection(DataCamera *d, const vec3 &_param, const vec3 &_pos0);
	string name(){	return "CameraMoveSelection";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Array<int> index_vel;
	Array<vec3> old_vel;
};

#endif /* ACTIONCAMERAMOVESELECTION_H_ */
