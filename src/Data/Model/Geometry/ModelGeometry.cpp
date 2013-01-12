/*
 * ModelGeometry.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "ModelGeometry.h"
#include "../DataModel.h"
#include "../ModelPolygon.h"
#include "../SkinGenerator.h"
#include "../../../lib/nix/nix.h"


static float Bernstein3(int i, float t)
{
	float ti = 1 - t;
	if (i == 0)
		return ti*ti*ti;
	if (i == 1)
		return 3*t*ti*ti;
	if (i == 2)
		return 3*t*t*ti;
	return t*t*t;
}


void ModelGeometry::clear()
{
	Polygon.clear();
	Vertex.clear();
}

void ModelGeometry::AddVertex(const vector &pos)
{
	ModelVertex v;
	v.pos = pos;
	Vertex.add(v);
}

void ModelGeometry::AddPolygon(Array<int> &v, Array<vector> &sv)
{
	ModelPolygon p;
	p.Side.resize(v.num);
	for (int k=0; k<v.num; k++){
		p.Side[k].Vertex = v[k];
		for (int l=0; l<MODEL_MAX_TEXTURES; l++)
			p.Side[k].SkinVertex[l] = sv[l*v.num + k];
	}
	p.Material = -1;
	p.NormalDirty = true;
	p.TriangulationDirty = true;
	Polygon.add(p);
}

void ModelGeometry::AddPolygonAutoTexture(Array<int> &v)
{
	SkinGenerator sg;
	sg.init_point_cloud_boundary(Vertex, v);

	Array<vector> sv;
	for (int l=0; l<MODEL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv.add(sg.get(Vertex[v[k]].pos));

	AddPolygon(v, sv);
}

void ModelGeometry::AddPolygonSingleTexture(Array<int> &v, Array<vector> &sv)
{
	Array<vector> sv2;
	for (int l=0; l<MODEL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv2.add(sv[k]);

	AddPolygon(v, sv2);
}

void ModelGeometry::AddBezier3(Array<vector> &v, int num_x, int num_y)
{
	int nv = Vertex.num;
	vector vv[4][4] = {{v[0], v[1], v[2], v[3]}, {v[4], v[5], v[6], v[7]}, {v[8], v[9], v[10], v[11]}, {v[12], v[13], v[14], v[15]}};
	for (int i=0; i<=num_x; i++)
		for (int j=0; j<=num_y; j++){
			float ti = (float)i / (float)num_x;
			float tj = (float)j / (float)num_y;
			vector p = v_0;
			for (int k=0; k<=3; k++)
				for (int l=0; l<=3; l++)
					p += Bernstein3(k, ti) * Bernstein3(l, tj) * vv[k][l];
			AddVertex(p);
		}
	for (int i=0; i<num_x; i++)
		for (int j=0; j<num_y; j++){
			Array<int> vv;
			vv.add(nv + i*(num_y+1)+j);
			vv.add(nv + i*(num_y+1)+j+1);
			vv.add(nv + (i+1)*(num_y+1)+j+1);
			vv.add(nv + (i+1)*(num_y+1)+j);
			Array<vector> sv;
			sv.add(vector((float) i    / (float)num_y, (float) j    / (float)num_y, 0));
			sv.add(vector((float) i    / (float)num_y, (float)(j+1) / (float)num_y, 0));
			sv.add(vector((float)(i+1) / (float)num_y, (float)(j+1) / (float)num_y, 0));
			sv.add(vector((float)(i+1) / (float)num_y, (float) j    / (float)num_y, 0));
			AddPolygonSingleTexture(vv, sv);
		}
}

void ModelGeometry::Add5(int nv, int v0, int v1, int v2, int v3, int v4)
{
	Array<int> v;
	v.add(nv + v0);
	v.add(nv + v1);
	v.add(nv + v2);
	v.add(nv + v3);
	v.add(nv + v4);
	AddPolygonAutoTexture(v);
}

void ModelGeometry::Add4(int nv, int v0, int v1, int v2, int v3)
{
	Array<int> v;
	v.add(nv + v0);
	v.add(nv + v1);
	v.add(nv + v2);
	v.add(nv + v3);
	AddPolygonAutoTexture(v);
}

void ModelGeometry::Add3(int nv, int v0, int v1, int v2)
{
	Array<int> v;
	v.add(nv + v0);
	v.add(nv + v1);
	v.add(nv + v2);
	AddPolygonAutoTexture(v);
}

void ModelGeometry::Add(ModelGeometry& geo)
{
}

void ModelGeometry::Weld(float epsilon)
{
	//return; // TODO
	//msg_write("------------------------ weld");
	float ep2 = epsilon * epsilon;
	for (int i=Vertex.num-2; i>=0; i--)
		for (int j=Vertex.num-1; j>i; j--)
			if ((Vertex[i].pos - Vertex[j].pos).length_sqr() < ep2){
				msg_write(format("del %d %d", i, j));
				/*bool allowed = true;
				foreach(ModelPolygon &p, Polygon){
					bool use_i = false;
					bool use_j = false;
					for (int k=0; k<p.Side.num; k++){
						use_i |= (p.Side[k].Vertex == i);
						use_j |= (p.Side[k].Vertex == j);
					}
					allowed &= (!use_i || !use_j);
				}
				if (!allowed)
					continue;*/

				Vertex.erase(j);

				// relink polygons
				foreach(ModelPolygon &p, Polygon)
					for (int k=0; k<p.Side.num; k++){
						if (p.Side[k].Vertex == j)
							p.Side[k].Vertex = i;
						else if (p.Side[k].Vertex > j)
							p.Side[k].Vertex --;
					}
			}
}

void ModelGeometry::Preview(int vb) const
{
	NixVBClear(vb);
	foreach(ModelPolygon &p, const_cast<Array<ModelPolygon>&>(Polygon)){
		Array<int> v = p.Triangulate(Vertex);
		vector n = p.GetNormal(Vertex);
		for (int i=0; i<v.num; i+=3)
			NixVBAddTria(vb, Vertex[p.Side[v[i]].Vertex].pos, n, 0,0,
					Vertex[p.Side[v[i+1]].Vertex].pos, n, 0,0,
					Vertex[p.Side[v[i+2]].Vertex].pos, n, 0,0);
	}
}

