/*
 * ActionWorldRotateObjects.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDROTATEOBJECTS_H_
#define ACTIONWORLDROTATEOBJECTS_H_

#include "../ActionMultiView.h"
class DataWorld;
class vector;

class ActionWorldRotateObjects: public ActionMultiView
{
public:
	ActionWorldRotateObjects(DataWorld *d, const vector &_param, const vector &_pos0);
	virtual ~ActionWorldRotateObjects();
	string name(){	return "WorldRotateObjects";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Array<vector> old_ang;
};

#endif /* ACTIONWORLDROTATEOBJECTS_H_ */
