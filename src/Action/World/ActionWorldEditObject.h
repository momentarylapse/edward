/*
 * ActionWorldEditObject.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDEDITOBJECT_H_
#define ACTIONWORLDEDITOBJECT_H_

#include "../Action.h"
#include "../../Data/World/DataWorld.h"

class ActionWorldEditObject : public Action
{
public:
	ActionWorldEditObject(int _index, const WorldObject &_data);
	virtual ~ActionWorldEditObject();
	string name(){	return "WorldEditObject";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	WorldObject data;
	int index;
};

#endif /* ACTIONWORLDEDITOBJECT_H_ */
