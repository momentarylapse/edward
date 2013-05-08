/*
 * ActionWorldAddObject.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDADDOBJECT_H_
#define ACTIONWORLDADDOBJECT_H_

#include "../Action.h"
#include "../../lib/math/math.h"

class ActionWorldAddObject : public Action
{
public:
	ActionWorldAddObject(const string &_filename, const vector &_pos, const vector &_ang = v_0);
	virtual ~ActionWorldAddObject();
	string name(){	return "WorldAddObject";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	string filename;
	vector pos;
	vector ang;
};

#endif /* ACTIONWORLDADDOBJECT_H_ */
