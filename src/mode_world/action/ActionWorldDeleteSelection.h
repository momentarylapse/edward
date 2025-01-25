/*
 * ActionWorldDeleteSelection.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETESELECTION_H_
#define ACTIONWORLDDELETESELECTION_H_

#include <action/Action.h>
#include "../data/DataWorld.h"
#include "../data/WorldObject.h"
#include "../data/WorldTerrain.h"
#include "../data/WorldCamera.h"
#include "../data/WorldLight.h"
#include "../data/WorldLink.h"

class ActionWorldDeleteSelection : public Action {
public:
	explicit ActionWorldDeleteSelection(DataWorld* d, const Data::Selection& selection);
	string name() override { return "WorldDeleteSelection";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

	Array<WorldObject> objects;
	Array<int> object_indices;

	Array<WorldTerrain> terrains;
	Array<int> terrain_indices;

	Array<WorldCamera> cameras;
	Array<int> camera_indices;

	Array<WorldLight> lights;
	Array<int> light_indices;

	Array<WorldLink> links;
	Array<int> link_indices;
};

#endif /* ACTIONWORLDDELETESELECTION_H_ */
