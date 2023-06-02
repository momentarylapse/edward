/*
 * ActionWorldTerrainBrushExtrude.cpp
 *
 *  Created on: 01.09.2013
 *      Author: michi
 */

#include "ActionWorldTerrainBrushExtrude.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldTerrain.h"
#include "../../../y/Terrain.h"

ActionWorldTerrainBrushExtrude::ActionWorldTerrainBrushExtrude(int _index, const vec3 &_pos, float _radius, float _depth) {
	index = _index;
	pos = _pos;
	radius = _radius;
	depth = _depth;
}

void* ActionWorldTerrainBrushExtrude::execute(Data* d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	auto tt = w->terrains[index];
	Terrain *t = tt.terrain;

	float r2 = radius * radius;

	for (int i=0;i<t->vertex.num;i++){
		float d2 = (pos - t->vertex[i] - tt.pos).length_sqr();
		if (d2 < r2 * 2){
			v_index.add(i);
			old_height.add(t->height[i]);
			t->height[i] += depth * exp(- d2 / r2 * 2);
			//t->vertex[i].y =  t->pos.y + t->height[i];
		}
	}
	t->update(-1, -1, -1, -1, -1);

	return NULL;
}

void ActionWorldTerrainBrushExtrude::undo(Data* d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	Terrain *t = w->terrains[index].terrain;

	foreachi(int vi, v_index, ii)
		t->height[vi] = old_height[ii];
	t->update(-1, -1, -1, -1, -1);
}
