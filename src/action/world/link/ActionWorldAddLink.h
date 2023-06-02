/*
 * ActionWorldAddLink.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_ACTION_WORLD_LINK_ACTIONWORLDADDLINK_H_
#define SRC_ACTION_WORLD_LINK_ACTIONWORLDADDLINK_H_

#include "../../Action.h"
#include "../../../data/world/WorldLink.h"

class ActionWorldAddLink : public Action {
public:
	ActionWorldAddLink(WorldLink &l);
	string name() { return "WorldAddLink"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	WorldLink link;
};

#endif /* SRC_ACTION_WORLD_LINK_ACTIONWORLDADDLINK_H_ */
