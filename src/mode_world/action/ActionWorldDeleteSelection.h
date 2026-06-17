/*
 * ActionWorldDeleteSelection.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../data/DataWorld.h"
#include <y/world/LevelData.h>

class ActionWorldDeleteSelection : public history::Action {
public:
	explicit ActionWorldDeleteSelection(DataWorld* d, const Selection& selection);
	string name() const override { return "WorldDeleteSelection";	}

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

	LevelData temp;
	Array<int> entity_indices;
};
