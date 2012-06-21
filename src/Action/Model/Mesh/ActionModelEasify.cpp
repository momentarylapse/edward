/*
 * ActionModelEasify.cpp
 *
 *  Created on: 18.06.2012
 *      Author: michi
 */

#include "ActionModelEasify.h"
#include "Vertex/ActionModelAddVertex.h"
#include "Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "Surface/Helper/ActionModelSurfaceRelinkTriangle.h"
#include "../../../Edward.h"

static vector get_area(DataModel *m, ModeModelTriangle &t)
{
	return (m->Vertex[t.Vertex[1]].pos - m->Vertex[t.Vertex[0]].pos) ^ (m->Vertex[t.Vertex[2]].pos - m->Vertex[t.Vertex[0]].pos);
}

static int edge_other_vertex(ModeModelEdge &e, int v)
{
	if (e.Vertex[0] == v)
		return e.Vertex[1];
	if (e.Vertex[1] == v)
		return e.Vertex[0];
	msg_error("edge_other_vertex");
	return e.Vertex[0];
}

static float get_weight(DataModel *m, ModeModelSurface &s, ModeModelEdge &e)
{
	float w = 0;
	vector v = (m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos) / 2;

	// triangle plane change
	foreachi(s.Triangle, t, ti){
		if (ti == e.Triangle[0])
			continue;
		if ((e.RefCount > 1) && (ti == e.Triangle[1]))
			continue;
		for (int l=0;l<e.RefCount;l++)
			for (int k=0;k<3;k++)
				if (t.Vertex[k] == e.Vertex[l]){
					vector area = get_area(m, t);
					vector vv[3];
					for (int i=0;i<3;i++)
						vv[i] = (i == k) ? v : m->Vertex[t.Vertex[i]].pos;
					vector area2 = (vv[1] - vv[0]) ^ (vv[2] - vv[0]);
					w += VecLength(area ^ area2) / (VecLength(area) + VecLength(area2));
					//w += VecLength(area - area2);
				}
	}

	// edge length
	w += VecLengthSqr(m->Vertex[e.Vertex[0]].pos - m->Vertex[e.Vertex[1]].pos);
	return w;
}

ActionModelEasify::ActionModelEasify(DataModel *m, float factor)
{
	ed->multi_view_3d->ResetMessage3d();

	foreachi(m->Surface, s, si){

		// calculate edge weights
		Array<float> we;
		foreach(s.Edge, e){
			float w = get_weight(m, s, e);
			we.add(w);
		}

		foreachi(s.Edge, e, ei)
			if (e.RefCount == 1){
				// find all edges sharing a vertex with e
				Set<int> ee;
				foreach(s.Triangle, t)
					for (int k=0;k<3;k++)
						for (int l=0;l<2;l++)
							if (t.Vertex[k] == e.Vertex[l]){
								ee.add(t.Edge[k]);
								ee.add(t.Edge[(k+2)%3]);
							}

				// compute damage...
				foreach(ee, eee)
					if (eee != ei){
						vector nv = (m->Vertex[s.Edge[eee].Vertex[0]].pos + m->Vertex[s.Edge[eee].Vertex[1]].pos) / 2;

						vector area = (m->Vertex[s.Edge[ei].Vertex[0]].pos - nv) ^ (m->Vertex[s.Edge[ei].Vertex[1]].pos - nv);
						we[eee] += VecLength( area );
					}

			}


//		foreachi(s.Edge, e, i)
//			ed->multi_view_3d->AddMessage3d(f2s(we[i], 1), (m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos) / 2);

		//msg_write(fa2s(we));

		// remove least important
		int index = 0;
		float min = we[0];
		foreachi(we, w, i)
			if (w < min){
				min = w;
				index = i;
			}

		ModeModelEdge &e = s.Edge[index];
		int v[2] = {e.Vertex[0], e.Vertex[1]};
		AddSubAction(new ActionModelAddVertex((m->Vertex[v[0]].pos + m->Vertex[v[1]].pos) / 2), m);

		Set<int> tria;
		for (int k=0;k<e.RefCount;k++)
			tria.add(e.Triangle[k]);

		foreachb(tria, t)
			AddSubAction(new ActionModelSurfaceDeleteTriangle(si, t), m);

		foreachbi(s.Triangle, t, i)
			for (int k=0;k<3;k++)
				if ((t.Vertex[k] == v[0]) || (t.Vertex[k] == v[1])){
					int nv[3];
					for (int l=0;l<3;l++)
						nv[l] = (k == l) ? (m->Vertex.num - 1) : t.Vertex[l];
					AddSubAction(new ActionModelSurfaceRelinkTriangle(m, si, i, nv[0], nv[1], nv[2]), m);
				}

	}
}

ActionModelEasify::~ActionModelEasify()
{
}
