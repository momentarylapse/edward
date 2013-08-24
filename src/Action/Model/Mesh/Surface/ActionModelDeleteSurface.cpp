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
	ModelSurface *s = &m->Surface[index];
	for (int i=s->Polygon.num-1; i>=0; i--)
		AddSubAction(new ActionModelSurfaceDeletePolygon(index, i), m);
	for (int i=s->Vertex.num-1; i>=0; i--)
		AddSubAction(new ActionModelDeleteUnusedVertex(s->Vertex[i]), m);

	AddSubAction(new ActionModelDeleteEmptySurface(index), m);

	return NULL;
}
