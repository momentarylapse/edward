/*
 * ActionWorldDeleteTerrain.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETETERRAIN_H_
#define ACTIONWORLDDELETETERRAIN_H_

#include "../../Action.h"
#include "../../../Data/World/DataWorld.h"

class ActionWorldDeleteTerrain : public Action
{
public:
	ActionWorldDeleteTerrain(int index);
	virtual ~ActionWorldDeleteTerrain();
	string name(){	return "WorldDeleteTerrain";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	WorldTerrain terrain;
};

#endif /* ACTIONWORLDDELETETERRAIN_H_ */
