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
	s.TestSanity("inv prae");

	// flip polygons
	foreachi(ModelPolygon &t, s.Polygon, ti){
		for (int k=0;k<t.Side.num/2;k++){
			int kk = t.Side.num - k - 1;

			// swap vertices
			int v = t.Side[k].Vertex;
			t.Side[k].Vertex = t.Side[kk].Vertex;
			t.Side[kk].Vertex = v;

			// swap skin vertices
			for (int tl=0;tl<MATERIAL_MAX_TEXTURES;tl++){
				vector tv = t.Side[k].SkinVertex[tl];
				t.Side[k].SkinVertex[tl] = t.Side[kk].SkinVertex[tl];
				t.Side[kk].SkinVertex[tl] = tv;
			}

			// swap edges
			int e = t.Side[k].Edge;
			t.Side[k].Edge = t.Side[kk-1].Edge;
			t.Side[kk-1].Edge = e;
		}

		// mark for update
		t.NormalDirty = true;
	}

	// flip edges
	foreach(ModelEdge &e, s.Edge){
		// swap vertices
		int v = e.Vertex[0];
		e.Vertex[0] = e.Vertex[1];
		e.Vertex[1] = v;

		// relink sides
		for (int k=0;k<e.RefCount;k++){
			if (e.Side[k] < s.Polygon[e.Polygon[k]].Side.num - 1)
				e.Side[k] = s.Polygon[e.Polygon[k]].Side.num - 2 - e.Side[k];
			s.Polygon[e.Polygon[k]].Side[e.Side[k]].EdgeDirection = k;
		}
	}

	s.TestSanity("inv post");
}

void *ActionModelSurfaceInvert::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreach(int surface, surfaces){

		assert((surface >= 0) && (surface < m->Surface.num));

		InvertSurface(m->Surface[surface]);
	}

	return NULL;
}



void ActionModelSurfaceInvert::undo(Data *d)
{
	execute(d);
}


