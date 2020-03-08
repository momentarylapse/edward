/*
 * ActionWorldDeleteLink.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_ACTION_WORLD_LINK_ACTIONWORLDDELETELINK_H_
#define SRC_ACTION_WORLD_LINK_ACTIONWORLDDELETELINK_H_

#include "../../Action.h"
#include "../../../Data/World/WorldLink.h"

class ActionWorldDeleteLink : public Action {
public:
	ActionWorldDeleteLink(int index);
	string name() override { return "WorldDeleteLink"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldLink link;
};

#endif /* SRC_ACTION_WORLD_LINK_ACTIONWORLDDELETELINK_H_ */
