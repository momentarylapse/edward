/*
 * ActionWorldDeleteObject.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETEOBJECT_H_
#define ACTIONWORLDDELETEOBJECT_H_

#include "../../Action.h"
#include "../../../Data/World/DataWorld.h"

class ActionWorldDeleteObject : public Action
{
public:
	ActionWorldDeleteObject(int index);
	virtual ~ActionWorldDeleteObject();
	string name(){	return "WorldDeleteObject";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	WorldObject object;
};

#endif /* ACTIONWORLDDELETEOBJECT_H_ */
