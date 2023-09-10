/*
 * ActionModelEasify.cpp
 *
 *  Created on: 18.06.2012
 *      Author: michi
 */

#include "ActionModelEasify.h"
#include "edge/ActionModelCollapseEdge.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/ModelPolygon.h"
#include "../../../EdwardWindow.h"
#include "../../../multiview/MultiView.h"



static vec3 get_deformed_area(ModelMesh *m, ModelPolygon &t, int index, const vec3 &new_pos) {
	// Newell's method
	vec3 n = v_0;
	vec3 p1 = m->vertex[t.side.back().vertex].pos;
	if ((t.side.num - 1) == index)
		p1 = new_pos;
	for (int i=0; i<t.side.num; i++) {
		vec3 p0 = p1;
		if (i == index)
			p1 = new_pos;
		else
			p1 = m->vertex[t.side[i].vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	return n * 0.5f;
}

struct PolyRef {
	int poly;
	int side;
};

// all weights are of dimension [area]
static float get_weight(ModelMesh *m, ModelEdge &e, Array<Array<PolyRef> > &ref) {
	float w = 0;
	int a = e.vertex[0];
	int b = e.vertex[1];
	vec3 new_pos = (m->vertex[a].pos + m->vertex[b].pos) / 2;

	Array<PolyRef> rr;
	rr.append(ref[a]);
	rr.append(ref[b]);

	// polygon plane change
	for (int i=0;i<rr.num;i++) {
		// find all polygons sharing a vertex with <e>
		// ...but not containing <e>
		if (rr[i].poly == e.polygon[0])
			continue;
		if ((e.ref_count > 1) && (rr[i].poly == e.polygon[1]))
			continue;
		ModelPolygon &p = m->polygon[rr[i].poly];

		// how much does the plane change
		vec3 area = p.get_area_vector(m->vertex);
		vec3 area2 = get_deformed_area(m, p, rr[i].side, new_pos);
		w += vec3::cross(area, area2).length() / (area.length() + area2.length()) * 4;
	}

	// edge length
	w += (m->vertex[e.vertex[0]].pos - m->vertex[e.vertex[1]].pos).length_sqr();
	return w;
}

void ActionModelEasify::CalculateWeights(ModelMesh *m) {
	//ed->multi_view_3d->reset_message_3d();

	// find all polygon sides for each vertex
	Array<Array<PolyRef> > ref;
	ref.resize(m->vertex.num);

	foreachi(ModelPolygon &p, m->polygon, ti) {
		for (int k=0;k<p.side.num;k++) {
			PolyRef r;
			r.poly = ti;
			r.side = k;
			ref[p.side[k].vertex].add(r);
		}
	}


	// calculate edge weights
	for (ModelEdge &e: m->edge)
		e.weight = get_weight(m, e, ref);

	// correction for boundary edges
	foreachi(ModelEdge &e, m->edge, ei)
		if (e.ref_count == 1) {
			// find all edges sharing a vertex with e
			Array<PolyRef> rr;
			rr.append(ref[e.vertex[0]]);
			rr.append(ref[e.vertex[1]]);
			base::set<int> ee;
			for (int i=0;i<rr.num;i++) {
				ModelPolygon &p = m->polygon[rr[i].poly];
				int k = rr[i].side;
				ee.add(p.side[k].edge);
				ee.add(p.side[(k-1+p.side.num) % p.side.num].edge);
			}

			// compute damage...
			for (int eee: ee)
				if (eee != ei) {
					vec3 nv = (m->vertex[m->edge[eee].vertex[0]].pos + m->vertex[m->edge[eee].vertex[1]].pos) / 2;

					vec3 area = vec3::cross(m->vertex[m->edge[ei].vertex[0]].pos - nv, m->vertex[m->edge[ei].vertex[1]].pos - nv);
					m->edge[eee].weight +=  area.length();
				}

		}


//		foreachi(s.Edge, e, i)
//			ed->multi_view_3d->AddMessage3d(f2s(we[i], 1), (m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos) / 2);
}

bool ActionModelEasify::EasifyStep(ModelMesh *m) {
	CalculateWeights(m);

	int _edge = -1;
	// remove least important
	float min = 0;
	foreachi(ModelEdge &e, m->edge, ei)
		if ((e.weight < min) or (_edge < 0)) {
			min = e.weight;
			_edge = ei;
		}

	if (_edge >= 0) {
		/*ModelEdge _e = m->Surface[_surface].Edge[_edge];
		// distribute weights
		Array<int> ee;
		foreachi(ModelEdge &e, m->Surface[_surface].Edge, i)
			if ((e.Vertex[0] == _e.Vertex[0]) || (e.Vertex[1] == _e.Vertex[0]) || (e.Vertex[0] == _e.Vertex[1]) || (e.Vertex[1] == _e.Vertex[1]))
				if (i != _edge)
					ee.add(i);
		for (int ie: ee)
			m->Surface[_surface].Edge[ie].Weight += _e.Weight / ee.num;*/

		// remove
		addSubAction(new ActionModelCollapseEdge(_edge), m->model);
		return true;
	}
	return false;
}


ActionModelEasify::ActionModelEasify(float _factor) {
	factor = _factor;
}

void *ActionModelEasify::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	os::Timer t;

	//CalculateWeights(m);
	int n = (int)((float)m->edit_mesh->polygon.num * factor);
	while (m->edit_mesh->polygon.num > n)
		if (!EasifyStep(m->edit_mesh))
			break;

	float dt = t.get();
	msg_write(format("easify: %f", dt));
	return nullptr;
}
