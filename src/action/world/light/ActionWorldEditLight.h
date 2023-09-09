/*
 * ActionWorldEditLight.h
 *
 *  Created on: 9 Sept 2023
 *      Author: michi
 */

#ifndef SRC_ACTION_WORLD_LIGHT_ACTIONWORLDEDITLIGHT_H_
#define SRC_ACTION_WORLD_LIGHT_ACTIONWORLDEDITLIGHT_H_

#include "../../Action.h"
#include "../../../data/world/WorldLight.h"

class ActionWorldEditLight : public Action {
public:
	ActionWorldEditLight(int index, const WorldLight &l);
	string name() { return "WorldEditLight"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldLight light;
};

#endif /* SRC_ACTION_WORLD_LIGHT_ACTIONWORLDEDITLIGHT_H_ */
