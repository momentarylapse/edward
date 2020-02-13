/*
 * ActionModelSurfaceInvert.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceInvert.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelSurfaceInvert::ActionModelSurfaceInvert(const Set<int> &_poly, bool _consistent) {
	poly = _poly;
	consistent = _consistent;
}

bool ActionModelSurfaceInvert::was_trivial() {
	return poly.num == 0;
}

void ActionModelSurfaceInvert::invert_polygons(DataModel *m) {

	// flip polygons
	for (int ti: poly) {
		auto &t = m->polygon[ti];
		for (int k=0; k<t.side.num/2; k++) {
			int kk = t.side.num - k - 1;

			// swap vertices
			int v = t.side[k].vertex;
			t.side[k].vertex = t.side[kk].vertex;
			t.side[kk].vertex = v;

			// swap skin vertices
			for (int tl=0;tl<MATERIAL_MAX_TEXTURES;tl++){
				vector tv = t.side[k].skin_vertex[tl];
				t.side[k].skin_vertex[tl] = t.side[kk].skin_vertex[tl];
				t.side[kk].skin_vertex[tl] = tv;
			}

			// swap edges
			int e = t.side[k].edge;
			t.side[k].edge = t.side[kk-1].edge;
			t.side[kk-1].edge = e;
		}

		for (int k=0; k<t.side.num; k++)
			edges.add(t.side[k].edge);

		// mark for update
		t.normal_dirty = true;
	}

}

void ActionModelSurfaceInvert::invert_edges(DataModel *m) {
	// flip edges
	for (int ei: edges) {
		auto &e = m->edge[ei];
		// swap vertices
		int v = e.vertex[0];
		e.vertex[0] = e.vertex[1];
		e.vertex[1] = v;

		// relink sides
		for (int k=0;k<e.ref_count;k++) {
			if (e.side[k] < m->polygon[e.polygon[k]].side.num - 1)
				e.side[k] = m->polygon[e.polygon[k]].side.num - 2 - e.side[k];
			m->polygon[e.polygon[k]].side[e.side[k]].edge_direction = k;
		}
	}
}

void *ActionModelSurfaceInvert::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	if (!consistent)
		throw ActionException("inconsistent surface");

	invert_polygons(m);
	invert_edges(m);

	return NULL;
}



void ActionModelSurfaceInvert::undo(Data *d) {
	if (consistent)
		execute(d);
}


