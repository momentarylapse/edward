/*
 * ActionModelSurfaceCopy.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceCopy.h"
#include "helper/ActionModelAddEmptySurface.h"
#include "helper/ActionModelSurfaceAddPolygon.h"
#include "../vertex/ActionModelAddVertex.h"
#include "../../../../data/model/DataModel.h"

ActionModelSurfaceCopy::ActionModelSurfaceCopy(int _surface)
{
	surface = _surface;
#if 0
	int si = m->get_surf_no(s);
	int dv = m->vertex.num;

	// copy vertices
	for (int v, s->vertex){
		addSubAction(new ActionModelAddVertex(m->vertex[v].pos), m);
		//m->Vertex.back().Surface = m->Surface.num;
	}

	for (ModelPolygon &t, s->polygon){
		int v[3];
		for (int k=0;k<t.side.num;k++)
			foreachi(int vv, s->vertex, vi)
				if (vv == t.side[k].vertex)
					v[k] = dv + vi;
		addSubAction(new ActionModelAddTriangle(m, v[0], v[1], v[2], t.material, t.SkinVertex[0], t.SkinVertex[1], t.SkinVertex[2]), m);
	}
	s = &m->surface[si];
#endif

	// dumb copy
/*	ModeModelSurface *r = m->AddSurface();
	s = &m->Surface[si];
	*r = *s;
	r->Vertex.clear();
	foreachi(s->Vertex, vv, i)
		r->Vertex.add(i + dv);

	// correct triangles
	for (r->Triangle, t){
		for (int k=0;k<3;k++){
			t->Vertex[k] = s->Vertex.find(t->Vertex[k]) + dv;
			m->Vertex[t->Vertex[k]].RefCount ++;
		}
	}

	// correct edges
	for (r->Edge, e)
		for (int k=0;k<2;k++)
			e->Vertex[k] = s->Vertex.find(e->Vertex[k]) + dv;*/

	//return r;
}

ActionModelSurfaceCopy::~ActionModelSurfaceCopy()
{
}

void *ActionModelSurfaceCopy::compose(Data *d)
{
	msg_todo("ActionModelSurfaceCopy");
#if 0
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface s = m->surface[surface];

	int dv = m->vertex.num;

	// copy vertices
	for (int v: s.vertex)
		addSubAction(new ActionModelAddVertex(m->vertex[v].pos), m);

	int s_no = m->surface.num;
	ModelSurface *copy = (ModelSurface*)addSubAction(new ActionModelAddEmptySurface(), m);

	for (ModelPolygon &t: s.polygon){
		Array<int> v = t.getVertices();
		Array<vector> sv = t.getSkinVertices();
		for (int k=0;k<t.side.num;k++)
			foreachi(int vv, s.vertex, vi)
				if (vv == t.side[k].vertex)
					v[k] = dv + vi;
		addSubAction(new ActionModelSurfaceAddPolygon(s_no, v, t.material, sv), m);
	}

	for (ModelPolygon &cp: copy->polygon)
		cp.temp_normal = cp.getNormal(m->vertex);

	return copy;
#endif
	return NULL;
}
