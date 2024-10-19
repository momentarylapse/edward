/*
 * ActionWorldDeleteTerrain.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETETERRAIN_H_
#define ACTIONWORLDDELETETERRAIN_H_

#include "../../Action.h"
#include "../../../data/world/WorldTerrain.h"

class ActionWorldDeleteTerrain : public Action {
public:
	explicit ActionWorldDeleteTerrain(int index);
	string name() override {	return "WorldDeleteTerrain";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldTerrain terrain;
};

#endif /* ACTIONWORLDDELETETERRAIN_H_ */
