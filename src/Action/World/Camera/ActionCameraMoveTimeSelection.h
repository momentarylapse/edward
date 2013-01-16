/*
 * ActionCameraMoveTimeSelection.h
 *
 *  Created on: 02.10.2012
 *      Author: michi
 */

#ifndef ACTIONCAMERAMOVETIMESELECTION_H_
#define ACTIONCAMERAMOVETIMESELECTION_H_

#include "../../Action.h"
#include "../../../lib/base/set.h"
class DataCamera;
class vector;

class ActionCameraMoveTimeSelection: public Action
{
public:
	ActionCameraMoveTimeSelection(DataCamera *d, float _param, float _pos0);
	virtual ~ActionCameraMoveTimeSelection(){}
	string name(){	return "CameraMoveTimeSelection";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Set<int> index;
	float pos0;
	float param;
	Array<float> old_data;
};

#endif /* ACTIONCAMERAMOVETIMESELECTION_H_ */
