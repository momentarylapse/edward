/*
 * ActionModelCollapseEdge.cpp
 *
 *  Created on: 25.08.2012
 *      Author: michi
 */

#include "ActionModelCollapseEdge.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkTriangle.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelCollapseEdge::ActionModelCollapseEdge(DataModel *m, int _surface, int _edge)
{
	assert(_surface >= 0);
	assert(_surface < m->Surface.num);
	ModelSurface &s = m->Surface[_surface];

	assert(_edge >= 0);
	assert(_edge < s.Edge.num);
	ModelEdge &e = s.Edge[_edge];
	int v[2] = {e.Vertex[0], e.Vertex[1]};
	AddSubAction(new ActionModelAddVertex((m->Vertex[v[0]].pos + m->Vertex[v[1]].pos) / 2), m);

	Set<int> tria;
	for (int k=0;k<e.RefCount;k++)
		tria.add(e.Triangle[k]);

	foreachb(int t, tria)
		AddSubAction(new ActionModelSurfaceDeleteTriangle(_surface, t), m);

	foreachib(ModelTriangle &t, s.Triangle, i)
		for (int k=0;k<3;k++)
			if ((t.Vertex[k] == v[0]) || (t.Vertex[k] == v[1])){
				int nv[3];
				for (int l=0;l<3;l++)
					nv[l] = (k == l) ? (m->Vertex.num - 1) : t.Vertex[l];
				AddSubAction(new ActionModelSurfaceRelinkTriangle(m, _surface, i, nv[0], nv[1], nv[2]), m);
			}

	// TODO... delete obsolete vertices
}

ActionModelCollapseEdge::~ActionModelCollapseEdge()
{
}


