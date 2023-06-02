/*
 * ActionWorldTerrainEditHeight.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "ActionWorldTerrainEditHeight.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldTerrain.h"
#include "../../../y/Terrain.h"
#include <assert.h>

#define Index(x,z)		((x)*(t->num_z+1)+(z))

ActionWorldTerrainEditHeight::ActionWorldTerrainEditHeight(int _index, const Array<float> &_height, rect _dest) {
	index = _index;
	height = _height;
	dest = _dest;

	assert(index >= 0);
	assert(height.num == (dest.x2 - dest.x1) * (dest.y2 - dest.y1));
}

void *ActionWorldTerrainEditHeight::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	assert(index >= 0);
	assert(index < w->terrains.num);

	Terrain *t = w->terrains[index].terrain;
	assert(t);

	assert(dest.x1 >= 0);
	assert(dest.y1 >= 0);
	assert(dest.x2 <= t->num_x + 1);
	assert(dest.y2 <= t->num_z + 1);

	// swap height data
	int i = 0;
	for (int x=dest.x1;x<dest.x2;x++)
		for (int z=dest.y1;z<dest.y2;z++) {
			float temp = t->height[Index(x, z)];
			t->height[Index(x, z)] = height[i];
			height[i ++] = temp;
		}
	t->update(dest.x1, dest.x2 + 1, dest.y1, dest.y2, -1);
	return NULL;
}



void ActionWorldTerrainEditHeight::undo(Data *d) {
	execute(d);
}


