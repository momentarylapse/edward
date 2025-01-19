/*
 * ActionWorldPaste.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDPASTE_H_
#define ACTIONWORLDPASTE_H_

#include "../ActionGroup.h"
class DataWorld;
class WorldObject;
class WorldTerrain;
class WorldLight;
class WorldCamera;

class ActionWorldPaste : public ActionGroup {
public:
	ActionWorldPaste(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains, Array<WorldCamera> &cameras, Array<WorldLight> &lights);
	string name() { return "WorldPaste"; }

	void *compose(Data *d);
private:
	Array<WorldObject> &objects;
	Array<WorldTerrain> &terrains;
	Array<WorldCamera> cameras;
	Array<WorldLight> lights;
};

#endif /* ACTIONWORLDPASTE_H_ */
