/*
 * ActionModelAddEmptySurface.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelAddEmptySurface.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>

// creates an empty surfaces -> don't use alone!

ActionModelAddEmptySurface::ActionModelAddEmptySurface()
{
}

ActionModelAddEmptySurface::~ActionModelAddEmptySurface()
{
}

void ActionModelAddEmptySurface::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Surface.back().Vertex.num + m->Surface.back().Edge.num + m->Surface.back().Polygon.num == 0);

	// should be empty at this point
	m->Surface.pop();
}



void *ActionModelAddEmptySurface::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface *s = m->AddSurface();
	return s;
}


