/*
 * ActionWorldAddObject.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDADDOBJECT_H_
#define ACTIONWORLDADDOBJECT_H_

#include "../../Action.h"
#include "../../../Data/World/WorldObject.h"

class ActionWorldAddObject : public Action {
public:
	ActionWorldAddObject(const WorldObject &o);
	string name() { return "WorldAddObject"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	WorldObject object;
};

#endif /* ACTIONWORLDADDOBJECT_H_ */
