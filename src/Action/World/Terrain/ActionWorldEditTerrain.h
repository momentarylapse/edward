/*
 * ActionWorldEditTerrain.h
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDEDITTERRAIN_H_
#define ACTIONWORLDEDITTERRAIN_H_

#include "../../Action.h"
#include "../../../Data/World/WorldTerrain.h"

class ActionWorldEditTerrain : public Action {
public:
	ActionWorldEditTerrain(int _index, const WorldEditingTerrain &_data);
	string name() override { return "WorldEditTerrain"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	WorldEditingTerrain data;
	int index;
};

#endif /* ACTIONWORLDEDITTERRAIN_H_ */
