/*
 * ActionModelSurfaceRelinkTriangle.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceRelinkTriangle.h"
#include "ActionModelSurfaceDeleteTriangle.h"
#include "ActionModelSurfaceAddTriangle.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelSurfaceRelinkTriangle::ActionModelSurfaceRelinkTriangle(DataModel *m, int _surface, int _triangle, Array<int> &_v)
{
	// old triangle data
	ModelPolygon &t = m->Surface[_surface].Polygon[_triangle];
	assert(_v.num == t.Side.num);
	int material = t.Material;
	Array<vector> sv;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<t.Side.num;k++)
			sv.add(t.Side[k].SkinVertex[l]);

	// delete old triangle
	AddSubAction(new ActionModelSurfaceDeleteTriangle(_surface, _triangle), m);

	// create new triangle
	AddSubAction(new ActionModelSurfaceAddTriangle(_surface, _v, material, sv, _triangle), m);
}

ActionModelSurfaceRelinkTriangle::~ActionModelSurfaceRelinkTriangle()
{
}
