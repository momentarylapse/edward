/*
 * ActionWorldPaste.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include <y/world/LevelData.h>

class DataWorld;
struct WorldEntity;

class ActionWorldPaste : public history::Action {
public:
	explicit ActionWorldPaste(const LevelData& temp);
	string name() const override { return "WorldPaste"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	LevelData temp;
};

