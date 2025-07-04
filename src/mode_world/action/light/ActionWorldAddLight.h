/*
 * ActionWorldAddLight.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_ACTION_WORLD_LINK_ACTIONWORLDADDLIGHT_H_
#define SRC_ACTION_WORLD_LINK_ACTIONWORLDADDLIGHT_H_

#include "../../../action/Action.h"
#include "../../data/WorldLight.h"

class ActionWorldAddLight : public Action {
public:
	explicit ActionWorldAddLight(const WorldLight &l);
	string name() override { return "WorldAddLight"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	WorldLight light;
};

#endif /* SRC_ACTION_WORLD_LINK_ACTIONWORLDADDLIGHT_H_ */
