/*
 * ActionModelCollapseEdge.cpp
 *
 *  Created on: 25.08.2012
 *      Author: michi
 */

#include "ActionModelCollapseEdge.h"
#include "../Polygon/Helper/ActionModelPolygonRemoveVertex.h"
#include "../Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "../Vertex/Helper/ActionModelMoveVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelCollapseEdge::ActionModelCollapseEdge(int _surface, int _edge)
{
	surface = _surface;
	edge = _edge;
}

void *ActionModelCollapseEdge::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(surface >= 0);
	assert(surface < m->Surface.num);
	ModelSurface &s = m->Surface[surface];

	assert(edge >= 0);
	assert(edge < s.Edge.num);
	ModelEdge &e = s.Edge[edge];
	int v[2] = {e.Vertex[0], e.Vertex[1]};

	// move one edge vertex to center
	vector pos = (m->Vertex[v[0]].pos + m->Vertex[v[1]].pos) / 2;
	AddSubAction(new ActionModelMoveVertex(v[0], pos), m);

	// delete the polygons which will be trivial
	foreachib(ModelPolygon &t, s.Polygon, i){
		for (int k=0;k<t.Side.num;k++)
			if (t.Side[k].Vertex == v[1]){
				AddSubAction(new ActionModelPolygonRemoveVertex(surface, i, k), m);
				break;
			}
		_foreach_it_.update(); // TODO
	}

	// delete obsolete vertex
	AddSubAction(new ActionModelDeleteUnusedVertex(v[1]), m);

	return NULL;
}


