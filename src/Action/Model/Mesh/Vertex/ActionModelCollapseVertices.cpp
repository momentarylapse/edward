/*
 * ActionModelCollapseVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelCollapseVertices.h"
#include "Helper/ActionModelDeleteUnusedVertex.h"
#include "ActionModelAddVertex.h"
#include "../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkPolygon.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

void ActionModelCollapseVertices::CollapseVerticesInSurface(DataModel *m, ModelSurface &s, int surf)
{
	Set<int> vert;
	vector pos = v_0;
	foreach(int v, s.vertex)
		if (m->vertex[v].is_selected){
			pos += m->vertex[v].pos;
			vert.add(v);
		}
	if (vert.num < 2)
		return;
	pos /= vert.num;
#if 0
	// delete triangles between 2+ selected vertices
	foreachib(ModelPolygon &t, s.Polygon, ti){
		int n_sel = 0;
		for (int k=0;k<3;k++)
			if (vert.contains(t.vertex[k]))
				n_sel ++;
		if (n_sel > 1){
			AddSubAction(new ActionModelSurfaceDeleteTriangle(surf, ti), m);
			_foreach_it_.update(); // TODO
		}
	}

	// new vertex
	AddSubAction(new ActionModelAddVertex(pos), m);
	int new_vertex = m->vertex.num - 1;

	// re-link triangles with 1 selected vertex
	foreachib(ModelPolygon &t, s.Polygon, ti){
		Array<int> v;
		for (int k=0;k<t.Side.num;k++)
			v.add(t.Side[k].vertex);
		for (int k=0;k<t.Side.num;k++)
			if (vert.contains(t.Side[k].vertex)){
				v[k] = new_vertex;
				AddSubAction(new ActionModelSurfaceRelinkPolygon(surf, ti, v), m);
				_foreach_it_.update(); // TODO
			}
	}

	// delete old vertices
	foreachb(int v, vert)
		AddSubAction(new ActionModelDeleteUnusedVertex(v), m);
#endif
}

ActionModelCollapseVertices::ActionModelCollapseVertices()
{}

void *ActionModelCollapseVertices::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(ModelSurface &s, m->surface, si)
		CollapseVerticesInSurface(m, s, si);

	return NULL;
}

