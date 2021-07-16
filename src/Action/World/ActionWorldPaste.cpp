/*
 * ActionWorldPaste.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionWorldPaste.h"
#include "Object/ActionWorldAddObject.h"
#include "Terrain/ActionWorldAddTerrain.h"
//#include "Camera/
#include "Light/ActionWorldAddLight.h"
#include "../../Data/World/DataWorld.h"
#include "../../Data/World/WorldObject.h"
#include "../../Data/World/WorldTerrain.h"
#include "../../Data/World/WorldCamera.h"
#include "../../Data/World/WorldLight.h"


ActionWorldPaste::ActionWorldPaste(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains, Array<WorldCamera> &_cameras, Array<WorldLight> &_lights) :
	objects(_objects),
	terrains(_terrains),
	lights(_lights),
	cameras(_cameras)
{
}

void *ActionWorldPaste::compose(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->clear_selection();

	for (auto &o: objects)
		addSubAction(new ActionWorldAddObject(o), w);

	for (auto &t: terrains)
		addSubAction(new ActionWorldAddTerrain(t.pos, t.filename), w);

	//for (auto &c: cameras)
	//	addSubAction(new ActionWorldAddCamera(c), w);

	for (auto &l: lights)
		addSubAction(new ActionWorldAddLight(l), w);

	return NULL;
}

