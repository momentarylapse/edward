/*
 * ComponentSelectionDialog.h
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"


class ComponentSelectionDialog: public hui::Dialog {
public:
	ComponentSelectionDialog(hui::Window *parent, ScriptInstanceData &data);

	ScriptInstanceData *data;
	bool selected;
	Array<ScriptInstanceData> available;


	static bool choose(hui::Window *parent, ScriptInstanceData &data);
};
