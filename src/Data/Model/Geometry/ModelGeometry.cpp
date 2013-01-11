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

	AddPolygonSingleTexture(v, sv);
}

void ModelGeometry::AddPolygonSingleTexture(Array<int> &v, Array<vector> &sv)
{
	Array<vector> sv2;
	for (int l=0; l<MODEL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv2.add(sv[k]);

	AddPolygonSingleTexture(v, sv2);
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

