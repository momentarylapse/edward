/*
 * ActionModelCollapseEdge.cpp
 *
 *  Created on: 25.08.2012
 *      Author: michi
 */

#include "ActionModelCollapseEdge.h"
#include "../polygon/helper/ActionModelPolygonRemoveVertex.h"
#include "../surface/helper/ActionModelSurfaceRelinkPolygon.h"
#include "../vertex/helper/ActionModelDeleteUnusedVertex.h"
#include "../vertex/helper/ActionModelMoveVertex.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include <assert.h>

ActionModelCollapseEdge::ActionModelCollapseEdge(int _edge) {
	edge = _edge;
}

void *ActionModelCollapseEdge::compose(Data *d) {
	DataModel *mod = dynamic_cast<DataModel*>(d);
	auto *m = mod->edit_mesh;
	assert(edge >= 0);
	assert(edge < m->edge.num);
	ModelEdge &e = m->edge[edge];
	int v[2] = {e.vertex[0], e.vertex[1]};

	// move one edge vertex to center
	vec3 pos = (m->vertex[v[0]].pos + m->vertex[v[1]].pos) / 2;
	addSubAction(new ActionModelMoveVertex(v[0], pos), mod);

	// any polygon using this edge -> remove 1 vertex
	Array<int> poly, side;
	foreachib(auto &t, m->polygon, i) {
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].edge == edge) {
				// don't disturb the edges -> remove delayed
				poly.add(i);
				side.add((k + 1 - t.side[k].edge_direction) % t.side.num);
				break;
			}
	}
	foreachi(int p, poly, i)
		addSubAction(new ActionModelPolygonRemoveVertex(p, side[i]), mod);

	// polygon using old vertex -> relink
	foreachib(ModelPolygon &t, m->polygon, i) {
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].vertex == v[1]) {
				Array<int> vv = t.get_vertices();
				vv[k] = v[0];
				addSubAction(new ActionModelSurfaceRelinkPolygon(i, vv), mod);
				break;
			}
		_foreach_it_.update(); // TODO
	}

	// delete obsolete vertex
	addSubAction(new ActionModelDeleteUnusedVertex(v[1]), mod);

	return NULL;
}


