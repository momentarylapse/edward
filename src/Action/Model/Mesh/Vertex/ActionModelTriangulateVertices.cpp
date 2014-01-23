/*
 * ActionModelTriangulateVertices.cpp
 *
 *  Created on: Jan 23, 2014
 *      Author: ankele
 */

#include "ActionModelTriangulateVertices.h"
#include "../Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"

ActionModelTriangulateVertices::ActionModelTriangulateVertices()
{
}

int find_rand_sel(DataModel *m)
{
	foreachi(ModelVertex &v, m->Vertex, i)
		if (v.is_selected)
			return i;
	return -1;
}

int find_closest_sel(DataModel *m, int i0)
{
	float dmin = 0;
	int imin = -1;
	foreachi(ModelVertex &v, m->Vertex, i)
		if ((v.is_selected) && (i != i0)){
			float d = (m->Vertex[i].pos - m->Vertex[i0].pos).length_sqr();
			if ((d < dmin) || (imin < 0)){
				imin = i;
				dmin = d;
			}
		}
	return imin;
}

float circum_radius(const vector &p1, const vector &p2, const vector &p3, vector &m)
{
	vector a = p2 - p1;
	vector b = p3 - p1;
	vector n = a ^ b;
	n.normalize();
	vector a_ortho = a ^ n;
	a_ortho.normalize();
	float mu = VecDotProduct(b, b - a) / (2 * VecDotProduct(a_ortho, b));
	float r = sqrt(VecDotProduct(a, a) + mu*mu);
	m = p1 + a/2 + a_ortho * mu;
	return r;
}

int find_for_edge(DataModel *m, int i0, int i1, Set<int> used)
{
	float rmin = 0;
	int imin = -1;
	foreachi(ModelVertex &v, m->Vertex, i)
		if ((v.is_selected) && (!used.contains(i))){
			vector cm;
			float r = circum_radius(m->Vertex[i0].pos, m->Vertex[i1].pos, m->Vertex[i].pos, cm);
			ed->multi_view_3d->AddMessage3d("x", cm);

			// other vertices in ball?
			bool ok = true;
			foreachi(ModelVertex &vv, m->Vertex, ii)
				if ((vv.is_selected) && (ii != i) && (ii != i0) && (ii != i1)){
					if ((cm - vv.pos).length() < r/2){
						ok = false;
						break;
					}
				}

			// smallest ball?
			if ((ok) && ((r < rmin) || (imin < 0))){
				imin = i;
				rmin = r;
			}
		}
	return imin;
}

void ActionModelTriangulateVertices::add_tria(Data*d, int i0, int i1, int i2, Set<int> &used, Array<int> &boundary)
{
	Array<int> v;
	v.add(i0);
	v.add(i2);
	v.add(i1);
	msg_write(ia2s(v));
	AddSubAction(new ActionModelAddPolygonAutoSkin(v, 0), d);

	used.add(i2);

	for (int i=0;i<boundary.num;i++)
		if (boundary[i] == i1){
			boundary.insert(i2, i);
			break;
		}
}

void *ActionModelTriangulateVertices::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	Set<int> used;
	Array<int> boundary;

	int i0 = find_rand_sel(m);
	if (i0 < 0)
		throw ActionException("no selected vertex found");
	int i1 = find_closest_sel(m, i0);
	if (i1 < 0)
		throw ActionException("no 2 selected vertices found");
	used.add(i0);
	used.add(i1);
	boundary.add(i0);
	boundary.add(i1);
	msg_write("b:" + ia2s(boundary));
	int i2 = find_for_edge(m, i0, i1, used);
	if (i2 < 0)
		throw ActionException("no third selected vertex found");

	add_tria(d, i0, i1, i2, used, boundary);

	while(true){
		msg_write("b:" + ia2s(boundary));
		i0 = boundary[0];
		i1 = boundary[1];
		int i2 = find_for_edge(m, i0, i1, used);
		msg_write(i2);
		if (i2 < 0)
			break;
		add_tria(d, i0, i1, i2, used, boundary);
	}


	return NULL;
}

