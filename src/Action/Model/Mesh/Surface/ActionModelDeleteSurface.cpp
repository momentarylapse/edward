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
	Array<int> v = s->Vertex;
	for (int i=s->Polygon.num-1; i>=0; i--)
		AddSubAction(new ActionModelSurfaceDeletePolygon(index, i), m);
	for (int i=v.num-1; i>=0; i--)
		AddSubAction(new ActionModelDeleteUnusedVertex(v[i]), m);

	AddSubAction(new ActionModelDeleteEmptySurface(index), m);

	return NULL;
}
