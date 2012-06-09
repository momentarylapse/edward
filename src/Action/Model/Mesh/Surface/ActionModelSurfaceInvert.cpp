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

	ModeModelSurface *s = &m->Surface[surface];

	foreach(s->Triangle, t){
		int v = t.Vertex[0];
		t.Vertex[0] = t.Vertex[1];
		t.Vertex[1] = v;
		for (int tl=0;tl<MODEL_MAX_TEXTURES;tl++){
			vector tv = t.SkinVertex[tl][0];
			t.SkinVertex[tl][0] = t.SkinVertex[tl][1];
			t.SkinVertex[tl][1] = tv;
		}
	}
	return NULL;
}



void ActionModelSurfaceInvert::undo(Data *d)
{
	execute(d);
}


