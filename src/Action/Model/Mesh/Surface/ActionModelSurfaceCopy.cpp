/*
 * ActionModelSurfaceCopy.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceCopy.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Triangle/ActionModelAddTriangle.h"

ActionModelSurfaceCopy::ActionModelSurfaceCopy(DataModel *m, ModeModelSurface *&s)
{
	int si = m->get_surf_no(s);
	int dv = m->Vertex.num;

	// copy vertices
	foreach(s->Vertex, v){
		AddSubAction(new ActionModelAddVertex(m->Vertex[v].pos), m);
		//m->Vertex.back().Surface = m->Surface.num;
	}

	foreach(s->Triangle, t){
		int v[3];
		for (int k=0;k<3;k++)
			foreachi(s->Vertex, vv, vi)
				if (vv == t.Vertex[k])
					v[k] = dv + vi;
		AddSubAction(new ActionModelAddTriangle(m, v[0], v[1], v[2], t.Material, t.SkinVertex[0], t.SkinVertex[1], t.SkinVertex[2]), m);
	}
	s = &m->Surface[si];

	// dumb copy
/*	ModeModelSurface *r = m->AddSurface();
	s = &m->Surface[si];
	*r = *s;
	r->Vertex.clear();
	foreachi(s->Vertex, vv, i)
		r->Vertex.add(i + dv);

	// correct triangles
	foreach(r->Triangle, t){
		for (int k=0;k<3;k++){
			t->Vertex[k] = s->Vertex.find(t->Vertex[k]) + dv;
			m->Vertex[t->Vertex[k]].RefCount ++;
		}
	}

	// correct edges
	foreach(r->Edge, e)
		for (int k=0;k<2;k++)
			e->Vertex[k] = s->Vertex.find(e->Vertex[k]) + dv;*/

	//return r;
}

ActionModelSurfaceCopy::~ActionModelSurfaceCopy()
{
	// TODO Auto-generated destructor stub
}

void *ActionModelSurfaceCopy::execute_return(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	return &m->Surface.back();
}
