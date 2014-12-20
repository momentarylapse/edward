/*
 * ActionModelSurfaceInvert.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceInvert.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelSurfaceInvert::ActionModelSurfaceInvert(const Set<int> &_surfaces)
{
	surfaces = _surfaces;
}

ActionModelSurfaceInvert::~ActionModelSurfaceInvert()
{
}

bool ActionModelSurfaceInvert::was_trivial()
{
	return surfaces.num == 0;
}

void ActionModelSurfaceInvert::InvertSurface(ModelSurface &s)
{
	s.testSanity("inv prae");

	// flip polygons
	foreachi(ModelPolygon &t, s.polygon, ti){
		for (int k=0;k<t.side.num/2;k++){
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

		// mark for update
		t.normal_dirty = true;
	}

	// flip edges
	foreach(ModelEdge &e, s.edge){
		// swap vertices
		int v = e.vertex[0];
		e.vertex[0] = e.vertex[1];
		e.vertex[1] = v;

		// relink sides
		for (int k=0;k<e.ref_count;k++){
			if (e.side[k] < s.polygon[e.polygon[k]].side.num - 1)
				e.side[k] = s.polygon[e.polygon[k]].side.num - 2 - e.side[k];
			s.polygon[e.polygon[k]].side[e.side[k]].edge_direction = k;
		}
	}

	s.testSanity("inv post");
}

void *ActionModelSurfaceInvert::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreach(int surface, surfaces){

		assert((surface >= 0) && (surface < m->surface.num));

		InvertSurface(m->surface[surface]);
	}

	return NULL;
}



void ActionModelSurfaceInvert::undo(Data *d)
{
	execute(d);
}


