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

void ModelGeometry::Preview(NixVertexBuffer *vb, int num_textures) const
{
	vb->clear();
	foreach(ModelPolygon &p, const_cast<Array<ModelPolygon>&>(Polygon)){
		p.TriangulationDirty = true;
		p.AddToVertexBuffer(Vertex, vb, num_textures);
	}
}


int ModelGeometry::AddEdge(int a, int b, int tria, int side)
{
	foreachi(ModelEdge &e, Edge, i){
		if ((e.Vertex[0] == a) && (e.Vertex[1] == b)){
			throw GeometryException("the new polygon would have neighbors of opposite orientation");
			/*e.RefCount ++;
			msg_error("surface error? inverse edge");
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;*/
		}
		if ((e.Vertex[0] == b) && (e.Vertex[1] == a)){
			if (e.Polygon[0] == tria)
				throw GeometryException("the new polygon would contain the same edge twice");
			if (e.RefCount > 1)
				throw GeometryException("there would be more than 2 polygons sharing an egde");
			e.RefCount ++;
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;
		}
	}
	ModelEdge ee;
	ee.Vertex[0] = a;
	ee.Vertex[1] = b;
	ee.is_selected = false;
	ee.is_special = false;
	ee.IsRound = false;
	ee.RefCount = 1;
	ee.Polygon[0] = tria;
	ee.Side[0] = side;
	ee.Polygon[1] = -1;
	Edge.add(ee);
	return Edge.num - 1;
}

void ModelGeometry::UpdateTopology()
{
	// clear
	Edge.clear();

	// add all triangles
	foreachi(ModelPolygon &t, Polygon, ti){

		// edges
		for (int k=0;k<t.Side.num;k++){
			t.Side[k].Edge = AddEdge(t.Side[k].Vertex, t.Side[(k + 1) % t.Side.num].Vertex, ti, k);
			t.Side[k].EdgeDirection = Edge[t.Side[k].Edge].RefCount - 1;
		}
	}
	// closed?
	IsClosed = true;
	foreach(ModelEdge &e, Edge)
		if (e.RefCount != 2){
			IsClosed = false;
			break;
		}
}

bool ModelGeometry::IsInside(const vector &p) const
{
	// how often does a ray from p intersect the surface?
	int n = 0;
	Array<vector> v;
	foreach(ModelPolygon &t, *(Array<ModelPolygon>*)(&Polygon)){

		// plane test
		if (((p - Vertex[t.Side[0].Vertex].pos) * t.TempNormal > 0) == (t.TempNormal.x > 0))
			continue;

		// polygon data
		if (v.num < t.Side.num)
			v.resize(t.Side.num);
		for (int k=0;k<t.Side.num;k++)
			v[k] = Vertex[t.Side[k].Vertex].pos;

		// bounding box tests
		bool smaller = true;
		for (int k=0;k<t.Side.num;k++)
			if (v[k].x >= p.x)
				smaller = false;
		if (smaller)
			continue;

		smaller = true;
		for (int k=1;k<t.Side.num;k++){
			if ((v[0].y < p.y) !=  (v[k].y < p.y))
				smaller = false;
			if ((v[0].z < p.z) !=  (v[k].z < p.z))
				smaller = false;
		}
		if (smaller)
			continue;

		// real intersection
		vector col;
		if (t.TriangulationDirty)
			t.UpdateTriangulation(Vertex);
		for (int k=t.Side.num-2;k>=0;k--)
			if (LineIntersectsTriangle(v[t.Side[k].Triangulation[0]], v[t.Side[k].Triangulation[1]], v[t.Side[k].Triangulation[2]], p, p + e_x, col, false))
				if (col.x > p.x)
					n ++;
	}

	// even or odd?
	return ((n % 2) == 1);
}

void ModelGeometry::Invert()
{
	foreach(ModelPolygon &p, Polygon)
		p.Invert();
}

void ModelGeometry::RemoveUnusedVertices()
{
	foreach(ModelVertex &v, Vertex)
		v.RefCount = 0;
	foreach(ModelPolygon &p, Polygon)
		for (int i=0;i<p.Side.num;i++)
			Vertex[p.Side[i].Vertex].RefCount ++;
	foreachib(ModelVertex &v, Vertex, vi)
		if (v.RefCount == 0){
			Vertex.erase(vi);
			// correct vertex indices
			foreach(ModelPolygon &p, Polygon)
				for (int i=0;i<p.Side.num;i++)
					if (p.Side[i].Vertex > vi)
						p.Side[i].Vertex --;
		}
}

bool ModelGeometry::IsMouseOver(MultiViewWindow *win, vector &tp)
{
	foreach(ModelPolygon &p, Polygon){
		// care for the sense of rotation?
		if (p.TempNormal * win->GetDirection() > 0)
			return false;

		// project all points
		Array<vector> v;
		for (int k=0;k<p.Side.num;k++){
			vector pp = win->Project(Vertex[p.Side[k].Vertex].pos);
			if ((pp.z <= 0) or (pp.z >= 1))
				return false; // TODO
			v.add(pp);
		}

		// test all sub-triangles
		p.UpdateTriangulation(Vertex);
		vector M = win->multi_view->m;
		for (int k=p.Side.num-3; k>=0; k--){
			int a = p.Side[k].Triangulation[0];
			int b = p.Side[k].Triangulation[1];
			int c = p.Side[k].Triangulation[2];
			float f,g;
			GetBaryCentric(M, v[a], v[b], v[c], f, g);
			// cursor in triangle?
			if ((f>0)&&(g>0)&&(f+g<1)){
				vector va = Vertex[p.Side[a].Vertex].pos;
				vector vb = Vertex[p.Side[b].Vertex].pos;
				vector vc = Vertex[p.Side[c].Vertex].pos;
				tp = va+f*(vb-va)+g*(vc-va);
				return true;
			}
		}
	}
	return false;
}
