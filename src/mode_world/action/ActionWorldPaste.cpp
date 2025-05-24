/*
 * ActionWorldPaste.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionWorldPaste.h"
#include "entity/ActionWorldAddEntity.h"
#include "object/ActionWorldAddObject.h"
#include "terrain/ActionWorldAddTerrain.h"
//#include "camera/
#include "light/ActionWorldAddLight.h"
#include "../data/DataWorld.h"
#include "../data/WorldObject.h"
#include "../data/WorldTerrain.h"
#include "../data/WorldCamera.h"
#include "../data/WorldLight.h"


ActionWorldPaste::ActionWorldPaste(const DataWorld& temp) :
	entities(temp.entities)
{
}

void *ActionWorldPaste::compose(Data *d) {
	auto *w = dynamic_cast<DataWorld*>(d);

	for (const auto &o: entities)
		addSubAction(new ActionWorldAddEntity(o), w);

	return nullptr;
}

