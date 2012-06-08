/*
 * ActionModel__DeleteSurface.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModel__DeleteSurface.h"
#include "../../Data/Model/DataModel.h"
#include <assert.h>

// can only delete empty surfaces!

ActionModel__DeleteSurface::ActionModel__DeleteSurface(int _surface)
{
	surface = _surface;
}

ActionModel__DeleteSurface::~ActionModel__DeleteSurface()
{
}

void *ActionModel__DeleteSurface::execute(Data *d)
{
	msg_write("__del surf do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Surface[surface].Vertex.num + m->Surface[surface].Triangle.num == 0);

	is_physical = m->Surface[surface].IsPhysical;
	is_visible = m->Surface[surface].IsVisible;
	m->Surface.erase(surface);
	return NULL;
}



void ActionModel__DeleteSurface::undo(Data *d)
{
	msg_write("__del surf undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = m->AddSurface(surface);
	s->IsPhysical = is_physical;
	s->IsVisible = is_visible;
}


