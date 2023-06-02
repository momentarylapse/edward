/*
 * ActionWorldDeleteObject.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETEOBJECT_H_
#define ACTIONWORLDDELETEOBJECT_H_

#include "../../Action.h"
#include "../../../data/world/WorldObject.h"

class ActionWorldDeleteObject : public Action {
public:
	ActionWorldDeleteObject(int index);
	string name() override { return "WorldDeleteObject"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldObject object;
};

#endif /* ACTIONWORLDDELETEOBJECT_H_ */
