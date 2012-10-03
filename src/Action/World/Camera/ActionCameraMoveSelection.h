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
class vector;

class ActionCameraMoveSelection: public ActionMultiView
{
public:
	ActionCameraMoveSelection(DataCamera *d, const vector &_param, const vector &_pos0);
	virtual ~ActionCameraMoveSelection(){}
	string name(){	return "CameraMoveSelection";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Array<int> index_vel;
	Array<vector> old_vel;
};

#endif /* ACTIONCAMERAMOVESELECTION_H_ */
