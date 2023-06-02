/*
 * ComponentSelectionDialog.h
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../../lib/hui/hui.h"
#include "../../../data/world/DataWorld.h"


class ComponentSelectionDialog: public hui::Dialog {
public:
	using Callback = std::function<void(const ScriptInstanceData &component)>;

	ComponentSelectionDialog(hui::Window *parent, Callback on_select);

	Callback on_select;
	Array<ScriptInstanceData> available;


	static void choose(hui::Window *parent, Callback on_select);
};
