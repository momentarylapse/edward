/*
 * ActionModelSurfaceAddTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAddTriangle.h"
#include "../../../../../Data/Model/DataModel.h"

// might create a "disjoint" surface -> don't use alone!

ActionModelSurfaceAddTriangle::ActionModelSurfaceAddTriangle(int _surface, Array<int> &_v, int _material, Array<vector> &_sv, int _index)
{
	surface = _surface;
	index = _index;
	v = _v;
	material = _material;
	sv = _sv;

}

ActionModelSurfaceAddTriangle::~ActionModelSurfaceAddTriangle()
{
}

void ActionModelSurfaceAddTriangle::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	if (index >= 0)
		s.RemovePolygon(index);
	else
		s.RemovePolygon(s.Polygon.num -1);
}



void *ActionModelSurfaceAddTriangle::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	// add triangle
	s.AddPolygon(v, material, sv, index);

	if (index >= 0)
		return &s.Polygon[index];
	return &s.Polygon.back();
}


