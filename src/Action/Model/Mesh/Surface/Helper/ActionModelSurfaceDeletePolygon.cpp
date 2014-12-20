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

	ModelSurface &s = m->surface[surface];

	s.addPolygon(vertex, material, skin, index);
}



void *ActionModelSurfaceDeletePolygon::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->surface[surface];
	ModelPolygon &t = s.polygon[index];

	// save old data
	material = t.material;
	vertex = t.getVertices();
	skin = t.getSkinVertices();

	// erase
	s.removePolygon(index);

	return NULL;
}


