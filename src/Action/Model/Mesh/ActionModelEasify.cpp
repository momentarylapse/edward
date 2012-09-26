/*
 * ActionModelEasify.cpp
 *
 *  Created on: 18.06.2012
 *      Author: michi
 */

#include "ActionModelEasify.h"
#include "Edge/ActionModelCollapseEdge.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

static vector get_area(DataModel *m, ModelTriangle &t)
{
	return (m->Vertex[t.Vertex[1]].pos - m->Vertex[t.Vertex[0]].pos) ^ (m->Vertex[t.Vertex[2]].pos - m->Vertex[t.Vertex[0]].pos);
}

static int edge_other_vertex(ModelEdge &e, int v)
{
	if (e.Vertex[0] == v)
		return e.Vertex[1];
	if (e.Vertex[1] == v)
		return e.Vertex[0];
	msg_error("edge_other_vertex");
	return e.Vertex[0];
}

static float get_weight(DataModel *m, ModelSurface &s, ModelEdge &e)
{
	float w = 0;
	vector v = (m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos) / 2;

	// triangle plane change
	foreachi(ModelTriangle &t, s.Triangle, ti){
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
					w += (area ^ area2).length() / (area.length() + area2.length());
					//w += VecLength(area - area2);
				}
	}

	// edge length
	w += (m->Vertex[e.Vertex[0]].pos - m->Vertex[e.Vertex[1]].pos).length_sqr();
	return w;
}

bool ActionModelEasify::EasifyStep(DataModel *m)
{
	ed->multi_view_3d->ResetMessage3d();

	foreachi(ModelSurface &s, m->Surface, si){

		// calculate edge weights
		foreach(ModelEdge &e, s.Edge)
			e.Weight = get_weight(m, s, e);

		foreachi(ModelEdge &e, s.Edge, ei)
			if (e.RefCount == 1){
				// find all edges sharing a vertex with e
				Set<int> ee;
				foreach(ModelTriangle &t, s.Triangle)
					for (int k=0;k<3;k++)
						for (int l=0;l<2;l++)
							if (t.Vertex[k] == e.Vertex[l]){
								ee.add(t.Edge[k]);
								ee.add(t.Edge[(k+2)%3]);
							}

				// compute damage...
				foreach(int eee, ee)
					if (eee != ei){
						vector nv = (m->Vertex[s.Edge[eee].Vertex[0]].pos + m->Vertex[s.Edge[eee].Vertex[1]].pos) / 2;

						vector area = (m->Vertex[s.Edge[ei].Vertex[0]].pos - nv) ^ (m->Vertex[s.Edge[ei].Vertex[1]].pos - nv);
						s.Edge[eee].Weight +=  area.length();
					}

			}


//		foreachi(s.Edge, e, i)
//			ed->multi_view_3d->AddMessage3d(f2s(we[i], 1), (m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos) / 2);
	}


	int _surface = 0, _edge = -1;
	// remove least important
	float min = 0;
	foreachi(ModelSurface &s, m->Surface, si)
		foreachi(ModelEdge &e, s.Edge, ei)
			if ((e.Weight < min) || (_edge < 0)){
				min = e.Weight;
				_surface = si;
				_edge = ei;
			}

	if (_edge >= 0){
		AddSubAction(new ActionModelCollapseEdge(m, _surface, _edge), m);
		return true;
	}
	return false;
}

ActionModelEasify::ActionModelEasify(DataModel *m, float factor)
{
	int n = (int)((float)m->GetNumTriangles() * factor);
	while(m->GetNumTriangles() > n)
		if (!EasifyStep(m))
			break;
}

ActionModelEasify::~ActionModelEasify()
{
}
