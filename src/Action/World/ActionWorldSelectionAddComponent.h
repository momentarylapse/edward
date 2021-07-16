/*
 * ActionWorldSelectionAddComponent.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#pragma once

#include "../Action.h"
#include "../../Data/World/DataWorld.h"

class ActionWorldSelectionAddComponent: public Action {
public:
	ActionWorldSelectionAddComponent(DataWorld *d, const ScriptInstanceData &com);
	string name() override { return "WorldSelectionAddComponent"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	ScriptInstanceData component;
	Array<int> index;
	Array<int> type;
};
