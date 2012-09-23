/*
 * ActionModelCollapseVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelCollapseVertices.h"
#include "Helper/ActionModelDeleteUnusedVertex.h"
#include "ActionModelAddVertex.h"
#include "../Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkTriangle.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

void ActionModelCollapseVertices::CollapseVerticesInSurface(DataModel *m, ModelSurface &s, int surf)
{
	Set<int> vert;
	vector pos = v_0;
	foreach(int v, s.Vertex)
		if (m->Vertex[v].is_selected){
			pos += m->Vertex[v].pos;
			vert.add(v);
		}
	if (vert.num < 2)
		return;
	pos /= vert.num;

	// delete triangles between 2+ selected vertices
	foreachib(ModelTriangle &t, s.Triangle, ti){
		int n_sel = 0;
		for (int k=0;k<3;k++)
			if (vert.contains(t.Vertex[k]))
				n_sel ++;
		if (n_sel > 1){
			AddSubAction(new ActionModelSurfaceDeleteTriangle(surf, ti), m);
			_foreach_it_.update(); // TODO
		}
	}

	// new vertex
	AddSubAction(new ActionModelAddVertex(pos), m);
	int new_vertex = m->Vertex.num - 1;

	// re-link triangles with 1 selected vertex
	foreachib(ModelTriangle &t, s.Triangle, ti){
		int v[3];
		for (int k=0;k<3;k++)
			v[k] = t.Vertex[k];
		for (int k=0;k<3;k++)
			if (vert.contains(t.Vertex[k])){
				v[k] = new_vertex;
				AddSubAction(new ActionModelSurfaceRelinkTriangle(m, surf, ti, v[0], v[1], v[2]), m);
				_foreach_it_.update(); // TODO
			}
	}

	// delete old vertices
	foreachb(int v, vert)
		AddSubAction(new ActionModelDeleteUnusedVertex(v), m);
}

ActionModelCollapseVertices::ActionModelCollapseVertices(DataModel *m)
{
	foreachi(ModelSurface &s, m->Surface, si)
		CollapseVerticesInSurface(m, s, si);
}

ActionModelCollapseVertices::~ActionModelCollapseVertices()
{
}

