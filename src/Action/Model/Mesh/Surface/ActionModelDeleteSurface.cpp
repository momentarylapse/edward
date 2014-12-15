/*
 * ActionModelDeleteSurface.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteSurface.h"
#include "../Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Helper/ActionModelSurfaceDeletePolygon.h"
#include "Helper/ActionModelDeleteEmptySurface.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelDeleteSurface::ActionModelDeleteSurface(int _index)
{
	index = _index;
}

ActionModelDeleteSurface::~ActionModelDeleteSurface()
{
}

void *ActionModelDeleteSurface::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface *s = &m->surface[index];
	Array<int> v = s->vertex;
	for (int i=s->polygon.num-1; i>=0; i--)
		addSubAction(new ActionModelSurfaceDeletePolygon(index, i), m);
	for (int i=v.num-1; i>=0; i--)
		addSubAction(new ActionModelDeleteUnusedVertex(v[i]), m);

	addSubAction(new ActionModelDeleteEmptySurface(index), m);

	return NULL;
}
