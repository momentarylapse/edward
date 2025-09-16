/*
 * ActionWorldPaste.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#pragma once

#include "../../action/Action.h"
#include <y/world/LevelData.h>

class DataWorld;
struct WorldEntity;

class ActionWorldPaste : public Action {
public:
	explicit ActionWorldPaste(const LevelData& temp);
	string name() override { return "WorldPaste"; }

	void* execute(Data *d) override;
	void undo(Data *d) override;
private:
	LevelData temp;
};

