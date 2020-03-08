/*
 * ActionWorldTerrainApplyHeightmap.h
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDTERRAINAPPLYHEIGHTMAP_H_
#define ACTIONWORLDTERRAINAPPLYHEIGHTMAP_H_

#include "../../ActionGroup.h"

class DataWorld;

class ActionWorldTerrainApplyHeightmap : public ActionGroup {
public:
	ActionWorldTerrainApplyHeightmap(DataWorld *data, const string &heightmap, float height_factor, float stretch_x, float stretch_z, const string &filter);
	string name(){	return "WorldTerrainApplyHeightmap";	}
};

#endif /* ACTIONWORLDTERRAINAPPLYHEIGHTMAP_H_ */
