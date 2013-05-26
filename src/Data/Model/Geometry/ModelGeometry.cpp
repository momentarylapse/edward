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
		for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
			p.Side[k].SkinVertex[l] = sv[l*v.num + k];
	}
	p.Material = -1;
	p.NormalDirty = true;
	p.TriangulationDirty = true;
	p.TempNormal = p.GetNormal(Vertex);
	for (int k=0;k<p.Side.num;k++)
		p.Side[k].Normal = p.TempNormal;
	Polygon.add(p);
}

void ModelGeometry::AddPolygonAutoTexture(Array<int> &v)
{
	SkinGenerator sg;
	sg.init_point_cloud_boundary(Vertex, v);

	Array<vector> sv;
	for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv.add(sg.get(Vertex[v[k]].pos));

	AddPolygon(v, sv);
}

void ModelGeometry::AddPolygonSingleTexture(Array<int> &v, Array<vector> &sv)
{
	Array<vector> sv2;
	for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv2.add(sv[k]);

	AddPolygon(v, sv2);
}

void ModelGeometry::AddBezier3(Array<vector> &v, int num_x, int num_y, float epsilon)
{
	vector vv[4][4] = {{v[0], v[1], v[2], v[3]}, {v[4], v[5], v[6], v[7]}, {v[8], v[9], v[10], v[11]}, {v[12], v[13], v[14], v[15]}};
	Array<vector> pp;
	Array<int> vn;
	vn.resize((num_x+1) * (num_y+1));
	bool merged_vertices = false;
	for (int i=0; i<=num_x; i++)
		for (int j=0; j<=num_y; j++){
			float ti = (float)i / (float)num_x;
			float tj = (float)j / (float)num_y;
			vector p = v_0;
			for (int k=0; k<=3; k++)
				for (int l=0; l<=3; l++)
					p += Bernstein3(k, ti) * Bernstein3(l, tj) * vv[k][l];
			int old = -1;
			if (epsilon > 0){
				foreachi(vector &vv, pp, ii)
					if ((p-vv).length_fuzzy() < epsilon)
						old = vn[ii];
			}
			if (old >= 0){
				merged_vertices = true;
				vn[i*(num_y+1)+j] = old;
			}else{
				vn[i*(num_y+1)+j] = Vertex.num;
				pp.add(p);
				AddVertex(p);
			}
		}
	for (int i=0; i<num_x; i++)
		for (int j=0; j<num_y; j++){
			Array<int> vv;
			vv.add(vn[ i   *(num_y+1)+j]);
			vv.add(vn[ i   *(num_y+1)+j+1]);
			vv.add(vn[(i+1)*(num_y+1)+j+1]);
			vv.add(vn[(i+1)*(num_y+1)+j]);
			Array<vector> sv;
			sv.add(vector((float) i    / (float)num_y, (float) j    / (float)num_y, 0));
			sv.add(vector((float) i    / (float)num_y, (float)(j+1) / (float)num_y, 0));
			sv.add(vector((float)(i+1) / (float)num_y, (float)(j+1) / (float)num_y, 0));
			sv.add(vector((float)(i+1) / (float)num_y, (float) j    / (float)num_y, 0));
			if (merged_vertices)
				for (int k=0;k<vv.num;k++)
					for (int kk=k+1;kk<vv.num;kk++)
						if (vv[k] == vv[kk]){
							vv.erase(kk);
							sv.erase(kk);
							kk --;
						}
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
	int nv = Vertex.num;
	int np = Polygon.num;
	Vertex.append(geo.Vertex);
	Polygon.append(geo.Polygon);
	for (int i=np; i<Polygon.num; i++)
		for (int k=0; k<Polygon[i].Side.num; k++)
			Polygon[i].Side[k].Vertex += nv;
}

void ModelGeometry::Weld(float epsilon)
{
	//return; // TODO
	//msg_write("------------------------ weld");
	float ep2 = epsilon * epsilon;
	for (int i=Vertex.num-2; i>=0; i--)
		for (int j=Vertex.num-1; j>i; j--)
			if ((Vertex[i].pos - Vertex[j].pos).length_sqr() < ep2){
				//msg_write(format("del %d %d", i, j));
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

void ModelGeometry::Weld(ModelGeometry &geo, float epsilon)
{
}

void ModelGeometry::Smoothen()
{
	Array<vector> n;
	n.resize(Vertex.num);

	// sum all normals (per vertex)
	foreach(ModelPolygon &p, Polygon){
		for (int k=0;k<p.Side.num;k++)
			n[p.Side[k].Vertex] += p.TempNormal;
	}

	// normalize
	for (int i=0;i<n.num;i++)
		n[i].normalize();

	// apply
	foreach(ModelPolygon &p, Polygon){
		for (int k=0;k<p.Side.num;k++)
			p.Side[k].Normal = n[p.Side[k].Vertex];
	}
}

void ModelGeometry::Transform(const matrix &mat)
{
	foreach(ModelVertex &v, Vertex)
		v.pos = mat * v.pos;
	matrix mat2 = mat * (float)pow(mat.determinant(), - 1.0f / 3.0f);
	foreach(ModelPolygon &p, Polygon){
		p.TempNormal = mat2.transform_normal(p.TempNormal);
		for (int k=0;k<p.Side.num;k++)
			p.Side[k].Normal = mat2.transform_normal(p.Side[k].Normal);
	}
}

void ModelGeometry::GetBoundingBox(vector &min, vector &max)
{
	if (Vertex.num > 0){
		min = max = Vertex[0].pos;
		foreach(ModelVertex &v, Vertex){
			min._min(v.pos);
			max._max(v.pos);
		}
	}else{
		min = max = v_0;
	}
}

void ModelGeometry::Preview(int vb, int num_textures) const
{
	NixVBClear(vb);
	foreach(ModelPolygon &p, const_cast<Array<ModelPolygon>&>(Polygon)){
		p.TriangulationDirty = true;
		p.AddToVertexBuffer(Vertex, vb, num_textures);
	}
}

