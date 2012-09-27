/*
 * ActionModelSurfaceRelinkPolygon.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceRelinkPolygon.h"
#include "ActionModelSurfaceDeleteTriangle.h"
#include "ActionModelSurfaceAddTriangle.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelSurfaceRelinkPolygon::ActionModelSurfaceRelinkPolygon(int _surface, int _polygon, Array<int> &_v) :
	v(_v)
{
	surface = _surface;
	polygon = _polygon;
}

void *ActionModelSurfaceRelinkPolygon::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// old triangle data
	ModelPolygon &t = m->Surface[surface].Polygon[polygon];
	assert(v.num == t.Side.num);
	int material = t.Material;
	Array<vector> sv;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<t.Side.num;k++)
			sv.add(t.Side[k].SkinVertex[l]);

	// delete old triangle
	AddSubAction(new ActionModelSurfaceDeleteTriangle(surface, polygon), m);

	// create new triangle
	AddSubAction(new ActionModelSurfaceAddTriangle(surface, v, material, sv, polygon), m);

	return NULL;
}
