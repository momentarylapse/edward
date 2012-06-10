/*
 * ActionModelSurfaceInvert.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceInvert.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelSurfaceInvert::ActionModelSurfaceInvert(int _surface)
{
	surface = _surface;
}

ActionModelSurfaceInvert::~ActionModelSurfaceInvert()
{
}

void *ActionModelSurfaceInvert::execute(Data *d)
{
	msg_write("surf invert do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert((surface >= 0) && (surface < m->Surface.num));

	ModeModelSurface &s = m->Surface[surface];

	// flip triangles
	foreachi(s.Triangle, t, ti){
		// swap vertices
		int v = t.Vertex[0];
		t.Vertex[0] = t.Vertex[1];
		t.Vertex[1] = v;

		// swap skin vertices
		for (int tl=0;tl<MODEL_MAX_TEXTURES;tl++){
			vector tv = t.SkinVertex[tl][0];
			t.SkinVertex[tl][0] = t.SkinVertex[tl][1];
			t.SkinVertex[tl][1] = tv;
		}

		// swap edges
		int e = t.Edge[1];
		t.Edge[1] = t.Edge[2];
		t.Edge[2] = e;

		// mark for update
		t.NormalDirty = true;
	}

	// flip edges
	foreach(s.Edge, e){
		int v = e.Vertex[0];
		e.Vertex[0] = e.Vertex[1];
		e.Vertex[1] = v;
	}

	return NULL;
}



void ActionModelSurfaceInvert::undo(Data *d)
{
	execute(d);
}


