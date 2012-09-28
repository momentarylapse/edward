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
	ModelSurface &s = m->Surface[surface];
	ModelPolygon &t = s.Polygon[poly];


	// save old polygon data
	Array<int> v;
	Array<vector> _sv;
	for (int k=0;k<t.Side.num;k++)
		v.add(t.Side[k].Vertex);
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<t.Side.num;k++)
			_sv.add(t.Side[k].SkinVertex[l]);
	int material = t.Material;

	// remove vertex
	v.erase(side);
	for (int l=MODEL_MAX_TEXTURES-1;l>=0;l--)
		_sv.erase(side + l * t.Side.num);

	// delete
	AddSubAction(new ActionModelSurfaceDeletePolygon(surface, poly), m);

	// recreate
	if (v.num > 2)
		AddSubAction(new ActionModelSurfaceAddPolygon(surface, v, material, _sv, poly), m);

	return NULL;
}

