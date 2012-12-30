/*
 * ActionModelSurfaceCopy.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceCopy.h"
#include "Helper/ActionModelAddEmptySurface.h"
#include "Helper/ActionModelSurfaceAddPolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelSurfaceCopy::ActionModelSurfaceCopy(int _surface)
{
	surface = _surface;
#if 0
	int si = m->get_surf_no(s);
	int dv = m->Vertex.num;

	// copy vertices
	foreach(int v, s->Vertex){
		AddSubAction(new ActionModelAddVertex(m->Vertex[v].pos), m);
		//m->Vertex.back().Surface = m->Surface.num;
	}

	foreach(ModelPolygon &t, s->Polygon){
		int v[3];
		for (int k=0;k<t.Side.num;k++)
			foreachi(int vv, s->Vertex, vi)
				if (vv == t.Side[k].Vertex)
					v[k] = dv + vi;
		AddSubAction(new ActionModelAddTriangle(m, v[0], v[1], v[2], t.Material, t.SkinVertex[0], t.SkinVertex[1], t.SkinVertex[2]), m);
	}
	s = &m->Surface[si];
#endif

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
}

void *ActionModelSurfaceCopy::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface s = m->Surface[surface];

	int dv = m->Vertex.num;

	// copy vertices
	foreach(int v, s.Vertex)
		AddSubAction(new ActionModelAddVertex(m->Vertex[v].pos), m);

	int s_no = m->Surface.num;
	ModelSurface *copy = (ModelSurface*)AddSubAction(new ActionModelAddEmptySurface(), m);

	foreach(ModelPolygon &t, s.Polygon){
		Array<int> v = t.GetVertices();
		Array<vector> sv = t.GetSkinVertices();
		for (int k=0;k<t.Side.num;k++)
			foreachi(int vv, s.Vertex, vi)
				if (vv == t.Side[k].Vertex)
					v[k] = dv + vi;
		AddSubAction(new ActionModelSurfaceAddPolygon(s_no, v, t.Material, sv), m);
	}

	foreach(ModelPolygon &cp, copy->Polygon)
		cp.TempNormal = cp.GetNormal(m);

	return copy;
}
