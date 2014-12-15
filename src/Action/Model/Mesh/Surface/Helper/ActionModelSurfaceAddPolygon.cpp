/*
 * ActionModelSurfaceAddPolygon.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAddPolygon.h"
#include "../../../../../Data/Model/DataModel.h"

// might create a "disjoint" surface -> don't use alone!

ActionModelSurfaceAddPolygon::ActionModelSurfaceAddPolygon(int _surface, Array<int> &_v, int _material, Array<vector> &_sv, int _index)
{
	surface = _surface;
	index = _index;
	v = _v;
	material = _material;
	sv = _sv;

}

ActionModelSurfaceAddPolygon::~ActionModelSurfaceAddPolygon()
{
}

void ActionModelSurfaceAddPolygon::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->surface[surface];

	if (index >= 0)
		s.RemovePolygon(index);
	else
		s.RemovePolygon(s.polygon.num -1);
}



void *ActionModelSurfaceAddPolygon::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->surface[surface];

	s.AddPolygon(v, material, sv, index);

	if (index >= 0)
		return &s.polygon[index];
	return &s.polygon.back();
}


