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
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Surface[surface].Vertex.num + m->Surface[surface].Polygon.num == 0);

	is_physical = m->Surface[surface].IsPhysical;
	is_visible = m->Surface[surface].IsVisible;
	m->Surface.erase(surface);

	// correct vertex/surface references
	foreach(ModelVertex &v, m->Vertex){
		assert(v.Surface != surface);
		if (v.Surface > surface)
			v.Surface --;
	}
	return NULL;
}



void ActionModelDeleteEmptySurface::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface *s = m->AddSurface(surface);
	s->IsPhysical = is_physical;
	s->IsVisible = is_visible;

	// correct vertex/surface references
	foreach(ModelVertex &v, m->Vertex)
		if (v.Surface >= surface)
			v.Surface ++;
}


