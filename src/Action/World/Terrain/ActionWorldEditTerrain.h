/*
 * ActionWorldEditTerrain.h
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDEDITTERRAIN_H_
#define ACTIONWORLDEDITTERRAIN_H_

#include "../../Action.h"
#include "../../../Data/World/DataWorld.h"

class ActionWorldEditTerrain : public Action
{
public:
	ActionWorldEditTerrain(int _index, const WorldEditingTerrain &_data);
	virtual ~ActionWorldEditTerrain();
	string name(){	return "WorldEditTerrain";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	WorldEditingTerrain data;
	int index;
};

#endif /* ACTIONWORLDEDITTERRAIN_H_ */
