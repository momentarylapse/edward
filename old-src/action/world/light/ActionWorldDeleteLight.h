/*
 * ActionWorldDeleteLight.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_ACTION_WORLD_LINK_ACTIONWORLDDELETELIGHT_H_
#define SRC_ACTION_WORLD_LINK_ACTIONWORLDDELETELIGHT_H_

#include "../../Action.h"
#include "../../../data/world/WorldLight.h"

class ActionWorldDeleteLight : public Action {
public:
	ActionWorldDeleteLight(int index);
	string name() override { return "WorldDeleteLight"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldLight light;
};

#endif /* SRC_ACTION_WORLD_LINK_ACTIONWORLDDELETELIGHT_H_ */
