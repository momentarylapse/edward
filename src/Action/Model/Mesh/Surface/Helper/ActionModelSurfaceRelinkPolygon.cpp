/*
 * ActionModelSurfaceRelinkPolygon.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceRelinkPolygon.h"
#include "ActionModelSurfaceDeletePolygon.h"
#include "ActionModelSurfaceAddPolygon.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelSurfaceRelinkPolygon::ActionModelSurfaceRelinkPolygon(int _surface, int _polygon, Array<int> &_v, int _new_surface) :
	v(_v)
{
	surface = _surface;
	polygon = _polygon;
	new_surface = _new_surface;
}

void *ActionModelSurfaceRelinkPolygon::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// old triangle data
	ModelPolygon &t = m->surface[surface].polygon[polygon];
	assert(v.num == t.side.num);
	int material = t.material;
	Array<vector> sv = t.getSkinVertices();

	// delete old triangle
	addSubAction(new ActionModelSurfaceDeletePolygon(surface, polygon), m);

	// create new triangle
	if (new_surface >= 0)
		addSubAction(new ActionModelSurfaceAddPolygon(new_surface, v, material, sv), m);
	else
		addSubAction(new ActionModelSurfaceAddPolygon(surface, v, material, sv, polygon), m);

	return NULL;
}
