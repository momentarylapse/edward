/*
 * ActionModelCollapseEdge.cpp
 *
 *  Created on: 25.08.2012
 *      Author: michi
 */

#include "ActionModelCollapseEdge.h"
#include "../Polygon/Helper/ActionModelPolygonRemoveVertex.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkPolygon.h"
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
	assert(surface < m->surface.num);
	ModelSurface &s = m->surface[surface];

	assert(edge >= 0);
	assert(edge < s.edge.num);
	ModelEdge &e = s.edge[edge];
	int v[2] = {e.vertex[0], e.vertex[1]};

	// move one edge vertex to center
	vector pos = (m->vertex[v[0]].pos + m->vertex[v[1]].pos) / 2;
	addSubAction(new ActionModelMoveVertex(v[0], pos), m);

	// any polygon using this edge -> remove 1 vertex
	Array<int> poly, side;
	foreachib(ModelPolygon &t, s.polygon, i){
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].edge == edge){
				// don't disturb the edges -> remove delayed
				poly.add(i);
				side.add((k + 1 - t.side[k].edge_direction) % t.side.num);
				break;
			}
	}
	foreachi(int p, poly, i)
		addSubAction(new ActionModelPolygonRemoveVertex(surface, p, side[i]), m);

	// polygon using old vertex -> relink
	foreachib(ModelPolygon &t, s.polygon, i){
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].vertex == v[1]){
				Array<int> vv = t.getVertices();
				vv[k] = v[0];
				addSubAction(new ActionModelSurfaceRelinkPolygon(surface, i, vv), m);
				break;
			}
		_foreach_it_.update(); // TODO
	}

	// delete obsolete vertex
	addSubAction(new ActionModelDeleteUnusedVertex(v[1]), m);

	return NULL;
}


