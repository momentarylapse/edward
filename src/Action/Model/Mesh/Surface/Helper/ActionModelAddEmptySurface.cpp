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
	assert(m->surface.back().vertex.num + m->surface.back().edge.num + m->surface.back().polygon.num == 0);

	// should be empty at this point
	m->surface.pop();
}



void *ActionModelAddEmptySurface::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface *s = m->AddSurface();
	return s;
}


