/*
 * ActionWorldEditObject.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDEDITOBJECT_H_
#define ACTIONWORLDEDITOBJECT_H_

#include "../../Action.h"
#include "../../../data/world/WorldObject.h"

class ActionWorldEditObject : public Action {
public:
	ActionWorldEditObject(int _index, const WorldObject &_data);
	string name() override {	return "WorldEditObject";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	WorldObject data;
	int index;
};

#endif /* ACTIONWORLDEDITOBJECT_H_ */
