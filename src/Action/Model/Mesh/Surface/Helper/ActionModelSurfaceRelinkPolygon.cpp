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
#include "../../../../../Data/Model/ModelMesh.h"
#include "../../../../../Data/Model/ModelPolygon.h"
#include <assert.h>

ActionModelSurfaceRelinkPolygon::ActionModelSurfaceRelinkPolygon(int _polygon, Array<int> &_v) :
	v(_v)
{
	polygon = _polygon;
}

void *ActionModelSurfaceRelinkPolygon::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	// old triangle data
	auto &t = m->edit_mesh->polygon[polygon];
	assert(v.num == t.side.num);
	int material = t.material;
	Array<vec3> sv = t.get_skin_vertices();

	// delete old triangle
	addSubAction(new ActionModelSurfaceDeletePolygon(polygon), m);

	// create new triangle
	addSubAction(new ActionModelSurfaceAddPolygon(v, material, sv, polygon), m);

	return NULL;
}
