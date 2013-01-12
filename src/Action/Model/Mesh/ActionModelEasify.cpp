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

#if 1
static vector get_area(DataModel *m, ModelPolygon &t)
{
	if (t.NormalDirty)
		t.UpdateTriangulation(m);
	vector a = v_0;
	for (int k=0;k<t.Side.num-2;k++){
		int va = t.Side[t.Side[k].Triangulation[0]].Vertex;
		int vb = t.Side[t.Side[k].Triangulation[1]].Vertex;
		int vc = t.Side[t.Side[k].Triangulation[2]].Vertex;
		a += (m->Vertex[vb].pos - m->Vertex[va].pos) ^ (m->Vertex[vc].pos - m->Vertex[va].pos);
	}
	return a;
}

static int edge_other_vertex(ModelEdge &e, int v)
{
	if (e.Vertex[0] == v)
		return e.Vertex[1];
	if (e.Vertex[1] == v)
		return e.Vertex[0];
	throw ActionException("edge_other_vertex");
	return e.Vertex[0];
}

static float get_weight(DataModel *m, ModelSurface &s, ModelEdge &e)
{
	float w = 0;
	vector v = (m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos) / 2;

	// triangle plane change
	foreachi(ModelPolygon &t, s.Polygon, ti){
		if (ti == e.Polygon[0])
			continue;
		if ((e.RefCount > 1) && (ti == e.Polygon[1]))
			continue;
		for (int l=0;l<e.RefCount;l++)
			for (int k=0;k<t.Side.num;k++)
				if (t.Side[k].Vertex == e.Vertex[l]){
					vector area = get_area(m, t);
					vector vv[3];
					for (int i=0;i<3;i++)
						vv[i] = (i == k) ? v : m->Vertex[t.Side[i].Vertex].pos;
					vector area2 = (vv[1] - vv[0]) ^ (vv[2] - vv[0]);
					w += (area ^ area2).length() / (area.length() + area2.length());
					//w += VecLength(area - area2);
				}
	}

	// edge length
	w += (m->Vertex[e.Vertex[0]].pos - m->Vertex[e.Vertex[1]].pos).length_sqr();
	return w;
}

void ActionModelEasify::CalculateWeights(DataModel *m)
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
				foreach(ModelPolygon &t, s.Polygon)
					for (int k=0;k<t.Side.num;k++)
						for (int l=0;l<2;l++)
							if (t.Side[k].Vertex == e.Vertex[l]){
								ee.add(t.Side[k].Edge);
								ee.add(t.Side[(k-1+t.Side.num) % t.Side.num].Edge);
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
}

bool ActionModelEasify::EasifyStep(DataModel *m)
{
	CalculateWeights(m);

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
		/*ModelEdge _e = m->Surface[_surface].Edge[_edge];
		// distribute weights
		Array<int> ee;
		foreachi(ModelEdge &e, m->Surface[_surface].Edge, i)
			if ((e.Vertex[0] == _e.Vertex[0]) || (e.Vertex[1] == _e.Vertex[0]) || (e.Vertex[0] == _e.Vertex[1]) || (e.Vertex[1] == _e.Vertex[1]))
				if (i != _edge)
					ee.add(i);
		foreach(int ie, ee)
			m->Surface[_surface].Edge[ie].Weight += _e.Weight / ee.num;*/

		// remove
		AddSubAction(new ActionModelCollapseEdge(_surface, _edge), m);
		return true;
	}
	return false;
}
#endif

ActionModelEasify::ActionModelEasify(float _factor)
{
	factor = _factor;
}

void *ActionModelEasify::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
#if 1
	//CalculateWeights(m);
	int n = (int)((float)m->GetNumPolygons() * factor);
	while(m->GetNumPolygons() > n)
		if (!EasifyStep(m))
			break;
#endif
	return NULL;
}

ActionModelEasify::~ActionModelEasify()
{
}
