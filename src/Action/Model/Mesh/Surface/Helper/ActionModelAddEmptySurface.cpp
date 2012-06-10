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
	msg_write("__add surf undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Surface.back().Vertex.num + m->Surface.back().Edge.num + m->Surface.back().Triangle.num == 0);

	// should be empty at this point
	m->Surface.pop();
}



void *ActionModelAddEmptySurface::execute(Data *d)
{
	msg_write("__add surf do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = m->AddSurface();
	return s;
}


