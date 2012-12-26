/*
 * ActionModelSurfaceDeletePolygon.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceDeletePolygon.h"
#include "../../../../../Data/Model/DataModel.h"

// might create an empty surface -> don't use alone!

ActionModelSurfaceDeletePolygon::ActionModelSurfaceDeletePolygon(int _surface, int _index)
{
	surface = _surface;
	index = _index;
}

ActionModelSurfaceDeletePolygon::~ActionModelSurfaceDeletePolygon()
{
}

void ActionModelSurfaceDeletePolygon::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	// add triangle
	try{
		s.AddPolygon(vertex, material, skin, index);
	}catch(GeometryException &e){
		throw ActionException(e.message);
	}
}



void *ActionModelSurfaceDeletePolygon::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];
	ModelPolygon &t = s.Polygon[index];

	// save old data
	material = t.Material;
	vertex = t.GetVertices();
	skin = t.GetSkinVertices();

	// erase
	try{
		s.RemovePolygon(index);
	}catch(GeometryException &e){
		throw ActionException(e.message);
	}

	return NULL;
}


