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
	ComponentSelectionDialog(Session *session, hui::Window *parent);

	base::promise<ScriptInstanceData> promise;
	Array<ScriptInstanceData> available;


	static base::future<ScriptInstanceData> choose(Session *session, hui::Window *parent);
};
