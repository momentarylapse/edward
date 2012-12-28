/*
 * ModelPolygon.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */



#include "DataModel.h"
#include "ModelPolygon.h"

vector ModelPolygon::GetNormal(const DataModel *m) const
{
	// Newell's method
	vector n = v_0;
	vector p1 = m->Vertex[Side.back().Vertex].pos;
	for (int i=0; i<Side.num; i++){
		vector p0 = p1;
		p1 = m->Vertex[Side[i].Vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	n.normalize();
	return n;
}

Array<int> ModelPolygon::GetVertices() const
{
	Array<int> v;
	v.resize(Side.num);
	for (int i=0; i<Side.num; i++)
		v[i] = Side[i].Vertex;
	return v;
}

Array<vector> ModelPolygon::GetSkinVertices() const
{
	Array<vector> sv;
	sv.resize(Side.num * MODEL_MAX_TEXTURES);
	int n = 0;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int i=0; i<Side.num; i++)
			sv[n ++] = Side[i].SkinVertex[l];
	return sv;
}


static float get_ang(const DataModel *m, int a, int b, int c, const vector &flat_n)
{
	vector v1 = m->Vertex[b].pos - m->Vertex[a].pos;
	vector v2 = m->Vertex[c].pos - m->Vertex[b].pos;
	v1.normalize();
	v2.normalize();
	float x = (v1 ^ v2) * flat_n;
	float y = v1 * v2;
	return atan2(x, y);
}

static bool vertex_in_tria(const DataModel *m, int a, int b, int c, int v)
{
	float f, g;
	GetBaryCentric(m->Vertex[v].pos, m->Vertex[a].pos, m->Vertex[b].pos, m->Vertex[c].pos, f, g);
	return ((f > 0) && (g > 0) && (f + g < 1));
}

/*static vector get_cloud_normal(DataModel *m, const Array<int> &v)
{
	Array<vector> p;
	for (int i=1;i<v.num;i++){
		p.add(m->Vertex[v[i]].pos - m->Vertex[v[0]].pos);
		p.back().normalize();
	}
	for (int i=0;i<p.num;i++)
		for (int j=i+1;j<p.num;j++){
			vector d = (p[i] ^ p[j]);
			float l = d.length();
			if (l > 0.1f)
				return d / l;
		}
	return v_0;
}*/

Array<int> ModelPolygon::Triangulate(const DataModel *m) const
{
	Array<int> output;

	Array<int> v, vi;
	for (int k=0;k<Side.num;k++){
		v.add(Side[k].Vertex);
		vi.add(k);
	}

	while(v.num > 3){

		// find largest angle (sharpest)
		// TODO: prevent colinear triangles!
		int i_max = 0;
		float f_max = 0;
		for (int i=0;i<v.num;i++){
			float f = get_ang(m, v[i], v[(i+1) % v.num], v[(i+2) % v.num], TempNormal);
			if (f < 0)
				continue;
			// cheat: ...
			float f_n = get_ang(m, v[(i+1) % v.num], v[(i+2) % v.num], v[(i+3) % v.num], TempNormal);
			float f_l = get_ang(m, v[(i-1+v.num) % v.num], v[i], v[(i+1) % v.num], TempNormal);
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

		output.add(vi[i_max]);
		output.add(vi[(i_max+1) % v.num]);
		output.add(vi[(i_max+2) % v.num]);

		v.erase((i_max+1) % v.num);
		vi.erase((i_max+1) % vi.num);
	}
	output.append(vi);

	return output;
}

void ModelPolygon::UpdateTriangulation(const DataModel *m)
{
	Array<int> v = Triangulate(m);
	for (int i=0; i<v.num; i+=3)
		for (int k=0; k<3; k++)
			Side[i/3].Triangulation[k] = v[i + k];
	TriangulationDirty = false;
}
