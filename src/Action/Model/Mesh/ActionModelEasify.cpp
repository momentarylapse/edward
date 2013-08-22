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
static vector get_deformed_area(DataModel *m, ModelPolygon &t, int index, const vector &new_pos)
{
	// Newell's method
	vector n = v_0;
	vector p1 = m->Vertex[t.Side.back().Vertex].pos;
	if ((t.Side.num - 1) == index)
		p1 = new_pos;
	for (int i=0; i<t.Side.num; i++){
		vector p0 = p1;
		if (i == index)
			p1 = new_pos;
		else
			p1 = m->Vertex[t.Side[i].Vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	return n * 0.5f;
}

struct PolyRef
{
	int poly;
	int side;
};

// all weights are of dimension [area]
static float get_weight(DataModel *m, ModelSurface &s, ModelEdge &e, Array<Array<PolyRef> > &ref)
{
	float w = 0;
	int a = e.Vertex[0];
	int b = e.Vertex[1];
	vector new_pos = (m->Vertex[a].pos + m->Vertex[b].pos) / 2;

	Array<PolyRef> rr;
	rr.append(ref[a]);
	rr.append(ref[b]);

	// polygon plane change
	for (int i=0;i<rr.num;i++){
		// find all polygons sharing a vertex with <e>
		// ...but not containing <e>
		if (rr[i].poly == e.Polygon[0])
			continue;
		if ((e.RefCount > 1) && (rr[i].poly == e.Polygon[1]))
			continue;
		ModelPolygon &p = s.Polygon[rr[i].poly];

		// how much does the plane change
		vector area = p.GetAreaVector(m->Vertex);
		vector area2 = get_deformed_area(m, p, rr[i].side, new_pos);
		w += (area ^ area2).length() / (area.length() + area2.length()) * 4;
	}

	// edge length
	w += (m->Vertex[e.Vertex[0]].pos - m->Vertex[e.Vertex[1]].pos).length_sqr();
	return w;
}

void ActionModelEasify::CalculateWeights(DataModel *m)
{
	ed->multi_view_3d->ResetMessage3d();

	foreachi(ModelSurface &s, m->Surface, si){

		// find all polygon sides for each vertex
		Array<Array<PolyRef> > ref;
		ref.resize(m->Vertex.num);

		foreachi(ModelPolygon &p, s.Polygon, ti){
			for (int k=0;k<p.Side.num;k++){
				PolyRef r;
				r.poly = ti;
				r.side = k;
				ref[p.Side[k].Vertex].add(r);
			}
		}


		// calculate edge weights
		foreach(ModelEdge &e, s.Edge)
			e.Weight = get_weight(m, s, e, ref);

		// correction for boundary edges
		foreachi(ModelEdge &e, s.Edge, ei)
			if (e.RefCount == 1){
				// find all edges sharing a vertex with e
				Array<PolyRef> rr;
				rr.append(ref[e.Vertex[0]]);
				rr.append(ref[e.Vertex[1]]);
				Set<int> ee;
				for (int i=0;i<rr.num;i++){
					ModelPolygon &p = s.Polygon[rr[i].poly];
					int k = rr[i].side;
					ee.add(p.Side[k].Edge);
					ee.add(p.Side[(k-1+p.Side.num) % p.Side.num].Edge);
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
	HuiTimer t;
#if 1
	//CalculateWeights(m);
	int n = (int)((float)m->GetNumPolygons() * factor);
	while(m->GetNumPolygons() > n)
		if (!EasifyStep(m))
			break;
#endif
	float dt = t.get();
	msg_write(format("easify: %f", dt));
	return NULL;
}

ActionModelEasify::~ActionModelEasify()
{
}
