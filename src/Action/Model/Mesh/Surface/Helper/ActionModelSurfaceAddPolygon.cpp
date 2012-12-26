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

	ModelSurface &s = m->Surface[surface];

	try{
		if (index >= 0)
			s.RemovePolygon(index);
		else
			s.RemovePolygon(s.Polygon.num -1);
	}catch(GeometryException &e){
		throw(ActionException("evil polygon: " + e.message));
	}
}



void *ActionModelSurfaceAddPolygon::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	// add triangle
	try{
		s.AddPolygon(v, material, sv, index);
	}catch(GeometryException &e){
		throw(ActionException("evil polygon: " + e.message));
	}

	if (index >= 0)
		return &s.Polygon[index];
	return &s.Polygon.back();
}


