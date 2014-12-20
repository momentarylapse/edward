/*
 * ActionModelPolygonRemoveVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelPolygonRemoveVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../../Surface/Helper/ActionModelSurfaceAddPolygon.h"

ActionModelPolygonRemoveVertex::ActionModelPolygonRemoveVertex(int _surface, int _poly, int _side)
{
	surface = _surface;
	poly = _poly;
	side = _side;
}

void *ActionModelPolygonRemoveVertex::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface &s = m->surface[surface];
	ModelPolygon &t = s.polygon[poly];


	// save old polygon data
	Array<int> v = t.getVertices();
	Array<vector> _sv = t.getSkinVertices();
	int material = t.material;

	// remove vertex
	v.erase(side);
	for (int l=MATERIAL_MAX_TEXTURES-1;l>=0;l--)
		_sv.erase(side + l * t.side.num);

	// delete
	addSubAction(new ActionModelSurfaceDeletePolygon(surface, poly), m);

	// recreate
	if (v.num > 2)
		addSubAction(new ActionModelSurfaceAddPolygon(surface, v, material, _sv, poly), m);

	return NULL;
}

