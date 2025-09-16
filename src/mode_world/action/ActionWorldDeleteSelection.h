/*
 * ActionWorldDeleteSelection.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#pragma once

#include <action/Action.h>
#include "../data/DataWorld.h"
#include "../data/WorldLink.h"
#include <y/world/LevelData.h>

class ActionWorldDeleteSelection : public Action {
public:
	explicit ActionWorldDeleteSelection(DataWorld* d, const Data::Selection& selection);
	string name() override { return "WorldDeleteSelection";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

	LevelData temp;
	Array<int> entity_indices;
	Array<int> link_indices;
};
