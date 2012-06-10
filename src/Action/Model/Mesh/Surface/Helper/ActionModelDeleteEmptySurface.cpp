/*
 * ActionModelDeleteEmptySurface.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteEmptySurface.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>

// can only delete empty surfaces!

ActionModelDeleteEmptySurface::ActionModelDeleteEmptySurface(int _surface)
{
	surface = _surface;
}

ActionModelDeleteEmptySurface::~ActionModelDeleteEmptySurface()
{
}

void *ActionModelDeleteEmptySurface::execute(Data *d)
{
	msg_write("__del surf do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Surface[surface].Vertex.num + m->Surface[surface].Triangle.num == 0);

	is_physical = m->Surface[surface].IsPhysical;
	is_visible = m->Surface[surface].IsVisible;
	m->Surface.erase(surface);
	return NULL;
}



void ActionModelDeleteEmptySurface::undo(Data *d)
{
	msg_write("__del surf undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = m->AddSurface(surface);
	s->IsPhysical = is_physical;
	s->IsVisible = is_visible;
}


