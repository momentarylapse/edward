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
	assert(m->surface[surface].vertex.num + m->surface[surface].polygon.num == 0);

	is_physical = m->surface[surface].is_physical;
	is_visible = m->surface[surface].is_visible;
	m->surface.erase(surface);

	// correct vertex/surface references
	foreach(ModelVertex &v, m->vertex){
		assert(v.surface != surface);
		if (v.surface > surface)
			v.surface --;
	}
	return NULL;
}



void ActionModelDeleteEmptySurface::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface *s = m->AddSurface(surface);
	s->is_physical = is_physical;
	s->is_visible = is_visible;

	// correct vertex/surface references
	foreach(ModelVertex &v, m->vertex)
		if (v.surface >= surface)
			v.surface ++;
}


