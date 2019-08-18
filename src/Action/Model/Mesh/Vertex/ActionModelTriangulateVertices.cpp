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

static int find_rand_sel(DataModel *m)
{
	foreachi(ModelVertex &v, m->vertex, i)
		if (v.is_selected)
			return i;
	return -1;
}

static int find_closest_sel(DataModel *m, int i0)
{
	float dmin = 0;
	int imin = -1;
	foreachi(ModelVertex &v, m->vertex, i)
		if ((v.is_selected) && (i != i0)){
			float d = (m->vertex[i].pos - m->vertex[i0].pos).length_sqr();
			if ((d < dmin) || (imin < 0)){
				imin = i;
				dmin = d;
			}
		}
	return imin;
}

static float circum_radius(const vector &p1, const vector &p2, const vector &p3, vector &m)
{
	vector a = p2 - p1;
	vector b = p3 - p1;
	vector n = a ^ b;
	if (n.length() < a.length_sqr() * 0.1f)
		return -1;
	n.normalize();
	vector a_ortho = a ^ n;
	a_ortho.normalize();
	float mu = vector::dot(b, b - a) / (2 * vector::dot(a_ortho, b));
	//float r = sqrt(vector::dot(a, a) + mu*mu);
	m = p1 + a/2 + a_ortho * mu;
	return (m - p1).length();//r;
}

static bool tria_ok(DataModel *m, int i0, int i1, int i2, float &r)
{
	vector cm;
	r = circum_radius(m->vertex[i0].pos, m->vertex[i1].pos, m->vertex[i2].pos, cm);
	if (r < 0)
		return false;

	// other vertices in ball?
	for (int ii=0; ii<m->vertex.num; ii++)
		if ((m->vertex[ii].is_selected) && (ii != i0) && (ii != i1) && (ii != i2)){
			if ((cm - m->vertex[ii].pos).length() < r){
				return false;
			}
		}
	return true;
}

static int find_for_edge(DataModel *m, int i0, int i1, Set<int> used)
{
	float rmin = 0;
	int imin = -1;
	foreachi(ModelVertex &v, m->vertex, i)
		if ((v.is_selected) && (!used.contains(i))){
			float r;
			if (!tria_ok(m, i0, i1, i, r))
				continue;

			// smallest ball?
			if ((r < rmin) || (imin < 0)){
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
	//msg_write(ia2s(v));
	addSubAction(new ActionModelAddPolygonAutoSkin(v, 0), d);

	used.add(i2);

	for (int i=0;i<boundary.num;i++)
		if (boundary[i] == i1){
			boundary.insert(i2, i);
			break;
		}
}

void ActionModelTriangulateVertices::add_tria2(Data*d, int i0, int i1, int i2, Array<int> &boundary)
{
	Array<int> v;
	v.add(i0);
	v.add(i1);
	v.add(i2);
	//msg_write(ia2s(v));
	addSubAction(new ActionModelAddPolygonAutoSkin(v, 0), d);

	for (int i=0;i<boundary.num;i++)
		if (boundary[i] == i1){
			boundary.erase(i);
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
	//msg_write("b:" + ia2s(boundary));
	int i2 = find_for_edge(m, i0, i1, used);
	if (i2 < 0)
		throw ActionException("no third selected vertex found");

	add_tria(d, i0, i1, i2, used, boundary);

	while(true){
		//msg_write("b:" + ia2s(boundary));
		bool ok = false;
		for (int i=0; i<boundary.num; i++){
			i0 = boundary[i];
			i1 = boundary[(i+1)%boundary.num];
			int i2 = find_for_edge(m, i0, i1, used);
			//msg_write(i2);
			if (i2 < 0)
				continue;
			add_tria(d, i0, i1, i2, used, boundary);
			ok = true;
			break;
		}
		if (ok)
			continue;
		for (int i=0; i<boundary.num-2; i++){
			i0 = boundary[i];
			i1 = boundary[(i+1)%boundary.num];
			i2 = boundary[(i+2)%boundary.num];
			float r;
			if (tria_ok(m, i0, i1, i2, r)){
				try{
					add_tria2(d, i0, i1, i2, boundary);
				}catch(...){
					return NULL;
				}
				ok = true;
				break;
			}
		}
		if (!ok)
			break;
	}


	return NULL;
}

