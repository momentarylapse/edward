/*
 * ActionModelPolygonAddVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelPolygonAddVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../../Surface/Helper/ActionModelSurfaceAddPolygon.h"

ActionModelPolygonAddVertex::ActionModelPolygonAddVertex(int _surface,
		int _poly, int _side, int _vertex, const vector* _sv)
{
	surface = _surface;
	poly = _poly;
	side = _side;
	vertex = _vertex;
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		sv[i] = _sv[i];
}

void* ActionModelPolygonAddVertex::compose(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface &s = m->surface[surface];
	ModelPolygon &t = s.polygon[poly];


	// save old polygon data
	Array<int> v = t.getVertices();
	Array<vector> _sv = t.getSkinVertices();
	int material = t.material;

	// insert vertex
	v.insert(vertex, side + 1);
	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		_sv.insert(sv[l], side + 1 + l * t.side.num);

	// delete
	addSubAction(new ActionModelSurfaceDeletePolygon(surface, poly), m);

	// recreate
	addSubAction(new ActionModelSurfaceAddPolygon(surface, v, material, _sv, poly), m);

	return NULL;
}


