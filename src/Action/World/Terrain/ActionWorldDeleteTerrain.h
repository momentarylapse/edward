/*
 * ActionWorldDeleteTerrain.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETETERRAIN_H_
#define ACTIONWORLDDELETETERRAIN_H_

#include "../../Action.h"
#include "../../../Data/World/WorldTerrain.h"

class ActionWorldDeleteTerrain : public Action {
public:
	ActionWorldDeleteTerrain(int index);
	string name(){	return "WorldDeleteTerrain";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldTerrain terrain;
};

#endif /* ACTIONWORLDDELETETERRAIN_H_ */
