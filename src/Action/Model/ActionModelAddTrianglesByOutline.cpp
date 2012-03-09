/*
 * ActionModelAddTrianglesByOutline.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ActionModelAddTrianglesByOutline.h"
#include "ActionModelAddVertex.h"
#include "ActionModelAddTriangle.h"
#include "../../Data/Model/DataModel.h"



//static float w_all;
static vector flat_n;

float get_ang(DataModel *m, int a, int b, int c)
{
	vector v1 = m->Vertex[b].pos - m->Vertex[a].pos;
	vector v2 = m->Vertex[c].pos - m->Vertex[b].pos;
	VecNormalize(v1);
	VecNormalize(v2);
	float x = (v1 ^ v2) * flat_n;
	float y = v1 * v2;
	return atan2(x, y);
}

bool vertex_in_tria(DataModel *m, int a, int b, int c, int v)
{
	float f, g;
	GetBaryCentric(m->Vertex[v].pos, m->Vertex[a].pos, m->Vertex[b].pos, m->Vertex[c].pos, f, g);
	return ((f > 0) && (g > 0) && (f + g < 1));
}

vector get_cloud_normal(DataModel *m, const Array<int> &v)
{
	Array<vector> p;
	for (int i=1;i<v.num;i++){
		p.add(m->Vertex[v[i]].pos - m->Vertex[v[0]].pos);
		VecNormalize(p.back());
	}
	for (int i=0;i<p.num;i++)
		for (int j=i+1;j<p.num;j++){
			vector d = (p[i] ^ p[j]);
			float l = VecLength(d);
			if (l > 0.1f)
				return d / l;
		}
	return v0;
}

void CreateTrianglesFlat(Array<Action*> *action, DataModel *m, Array<int> &v)
{
	if (v.num == 3){
		action->add(new ActionModelAddTriangle(v[0], v[1], v[2], v0, e_x, e_y));
	}else if (v.num > 3){

		flat_n = get_cloud_normal(m, v);
		//msg_write(format("%f  %f  %f", flat_n.x, flat_n.y, flat_n.z));
		// orientation

		float f_sum = 0;
		for (int i=0;i<v.num;i++)
			f_sum += get_ang(m, v[i], v[(i+1) % v.num], v[(i+2) % v.num]);
		//msg_write(f2s(f_sum, 3));
		if (f_sum < 0)
			flat_n = -flat_n;

		// find largest angle (sharpest)
		// TODO: prevent colinear triangles!
		int i_max = 0;
		float f_max = 0;
		for (int i=0;i<v.num;i++){
			float f = get_ang(m, v[i], v[(i+1) % v.num], v[(i+2) % v.num]);
			if (f < 0)
				continue;
			// cheat: ...
			float f_n = get_ang(m, v[(i+1) % v.num], v[(i+2) % v.num], v[(i+3) % v.num]);
			float f_l = get_ang(m, v[(i-1+v.num) % v.num], v[i], v[(i+1) % v.num]);
			if (f_n >= 0)
				f += 0.01f / (f_n + 0.01f);
			if (f_l >= 0)
				f += 0.01f / (f_l + 0.01f);

			if (f > f_max){
				// other vertices within this triangle?
				bool ok = true;
				for (int j=0;j<v.num;j++){
					if ((j == i) || (j == ((i+1) % v.num)) || (j == ((i+2) % v.num)))
						continue;
					if (vertex_in_tria(m, v[i], v[(i+1) % v.num], v[(i+2) % v.num], v[j])){
						ok = false;
						break;
					}
				}

				if (ok){
					f_max = f;
					i_max = i;
				}
			}
		}

		action->add(new ActionModelAddTriangle(v[i_max], v[(i_max+1) % v.num], v[(i_max+2) % v.num], v0, e_x, e_y));

		v.erase((i_max+1) % v.num);
		CreateTrianglesFlat(action, m, v);
	}
}

ActionModelAddTrianglesByOutline::ActionModelAddTrianglesByOutline(Array<int> vertex, DataModel *data)
{
	CreateTrianglesFlat(&action, data, vertex);
}

ActionModelAddTrianglesByOutline::~ActionModelAddTrianglesByOutline()
{
}
