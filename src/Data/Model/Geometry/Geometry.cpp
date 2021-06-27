/*
 * Geometry.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "Geometry.h"
#include "../DataModel.h"
#include "../ModelPolygon.h"
#include "../SkinGenerator.h"
#include "../../../lib/nix/nix.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"


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


void Geometry::clear()
{
	polygon.clear();
	vertex.clear();
}

void Geometry::add_vertex(const vector &pos)
{
	vertex.add(ModelVertex(pos));
}

void Geometry::add_polygon(const Array<int> &v, const Array<vector> &sv)
{
	ModelPolygon p;
	p.side.resize(v.num);
	for (int k=0; k<v.num; k++){
		p.side[k].vertex = v[k];
		for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
			p.side[k].skin_vertex[l] = sv[l*v.num + k];
	}
	p.material = -1;
	p.normal_dirty = true;
	p.triangulation_dirty = true;
	p.temp_normal = p.get_normal(vertex);
	for (int k=0;k<p.side.num;k++)
		p.side[k].normal = p.temp_normal;
	polygon.add(p);
}

void Geometry::add_polygon_auto_texture(const Array<int> &v)
{
	SkinGenerator sg;
	sg.init_point_cloud_boundary(vertex, v);

	Array<vector> sv;
	for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv.add(sg.get(vertex[v[k]].pos));

	add_polygon(v, sv);
}

void Geometry::add_polygon_single_texture(const Array<int> &v, const Array<vector> &sv)
{
	Array<vector> sv2;
	for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv2.add(sv[k]);

	add_polygon(v, sv2);
}

void Geometry::add_bezier3(const Array<vector> &v, int num_x, int num_y, float epsilon)
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
				vn[i*(num_y+1)+j] = vertex.num;
				pp.add(p);
				add_vertex(p);
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
			add_polygon_single_texture(vv, sv);
		}
}

void Geometry::add_easy(int nv, const Array<int> &delta) {
	Array<int> v;
	for (int d: delta)
		v.add(nv + d);
	add_polygon_auto_texture(v);
}

void Geometry::add(const Geometry& geo)
{
	int nv = vertex.num;
	int np = polygon.num;
	vertex.append(geo.vertex);
	polygon.append(geo.polygon);
	for (int i=np; i<polygon.num; i++)
		for (int k=0; k<polygon[i].side.num; k++)
			polygon[i].side[k].vertex += nv;
}

void Geometry::weld(float epsilon)
{
	//return; // TODO
	//msg_write("------------------------ weld");
	float ep2 = epsilon * epsilon;
	for (int i=vertex.num-2; i>=0; i--)
		for (int j=vertex.num-1; j>i; j--)
			if ((vertex[i].pos - vertex[j].pos).length_sqr() < ep2){
				//msg_write(format("del %d %d", i, j));
				/*bool allowed = true;
				foreach(ModelPolygon &p, Polygon){
					bool use_i = false;
					bool use_j = false;
					for (int k=0; k<p.Side.num; k++){
						use_i |= (p.Side[k].Vertex == i);
						use_j |= (p.Side[k].Vertex == j);
					}
					allowed &= (!use_i or !use_j);
				}
				if (!allowed)
					continue;*/

				vertex.erase(j);

				// relink polygons
				for (ModelPolygon &p: polygon)
					for (int k=0; k<p.side.num; k++){
						if (p.side[k].vertex == j)
							p.side[k].vertex = i;
						else if (p.side[k].vertex > j)
							p.side[k].vertex --;
					}
			}
}

void Geometry::weld(const Geometry &geo, float epsilon)
{
}

void Geometry::smoothen()
{
	Array<vector> n;
	n.resize(vertex.num);

	// sum all normals (per vertex)
	for (ModelPolygon &p: polygon){
		for (int k=0;k<p.side.num;k++)
			n[p.side[k].vertex] += p.temp_normal;
	}

	// normalize
	for (int i=0;i<n.num;i++)
		n[i].normalize();

	// apply
	for (ModelPolygon &p: polygon){
		for (int k=0;k<p.side.num;k++)
			p.side[k].normal = n[p.side[k].vertex];
	}
}

void Geometry::transform(const matrix &mat)
{
	for (ModelVertex &v: vertex)
		v.pos = mat * v.pos;
	//matrix mat2 = mat * (float)pow(mat.determinant(), - 1.0f / 3.0f);
	for (ModelPolygon &p: polygon){
		/*p.temp_normal = mat2.transform_normal(p.temp_normal);
		for (int k=0;k<p.side.num;k++)
			p.side[k].normal = mat2.transform_normal(p.side[k].normal);*/
		p.temp_normal = p.get_normal(vertex);
		for (int k=0;k<p.side.num;k++)
			p.side[k].normal = p.temp_normal;
	}
}

void Geometry::get_bounding_box(vector &min, vector &max)
{
	if (vertex.num > 0){
		min = max = vertex[0].pos;
		for (ModelVertex &v: vertex){
			min._min(v.pos);
			max._max(v.pos);
		}
	}else{
		min = max = v_0;
	}
}

void Geometry::build(nix::VertexBuffer *vb) const {
	VertexStagingBuffer vbs;
	int num_textures = vb->num_buffers - 2;
	for (auto &p: const_cast<Array<ModelPolygon>&>(polygon)){
		p.triangulation_dirty = true;
		p.add_to_vertex_buffer(vertex, vbs, num_textures);
	}
	vbs.build(vb, num_textures);
}


int Geometry::add_edge(int a, int b, int tria, int side)
{
	foreachi(ModelEdge &e, edge, i){
		if ((e.vertex[0] == a) and (e.vertex[1] == b)){
			throw GeometryException("the new polygon would have neighbors of opposite orientation");
			/*e.RefCount ++;
			msg_error("surface error? inverse edge");
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;*/
		}
		if ((e.vertex[0] == b) and (e.vertex[1] == a)){
			if (e.polygon[0] == tria)
				throw GeometryException("the new polygon would contain the same edge twice");
			if (e.ref_count > 1)
				throw GeometryException("there would be more than 2 polygons sharing an egde");
			e.ref_count ++;
			e.polygon[1] = tria;
			e.side[1] = side;
			return i;
		}
	}
	ModelEdge ee;
	ee.vertex[0] = a;
	ee.vertex[1] = b;
	ee.is_selected = false;
	ee.is_special = false;
	ee.is_round = false;
	ee.ref_count = 1;
	ee.polygon[0] = tria;
	ee.side[0] = side;
	ee.polygon[1] = -1;
	edge.add(ee);
	return edge.num - 1;
}

void Geometry::update_topology()
{
	// clear
	edge.clear();

	// add all triangles
	foreachi(ModelPolygon &t, polygon, ti){

		// edges
		for (int k=0;k<t.side.num;k++){
			t.side[k].edge = add_edge(t.side[k].vertex, t.side[(k + 1) % t.side.num].vertex, ti, k);
			t.side[k].edge_direction = edge[t.side[k].edge].ref_count - 1;
		}
	}
	// closed?
	is_closed = true;
	for (ModelEdge &e: edge)
		if (e.ref_count != 2){
			is_closed = false;
			break;
		}
}

bool Geometry::is_inside(const vector &p) const
{
	// how often does a ray from p intersect the surface?
	int n = 0;
	Array<vector> v;
	for (ModelPolygon &t: *(Array<ModelPolygon>*)(&polygon)){

		// plane test
		if ((vector::dot(p - vertex[t.side[0].vertex].pos, t.temp_normal) > 0) == (t.temp_normal.x > 0))
			continue;

		// polygon data
		if (v.num < t.side.num)
			v.resize(t.side.num);
		for (int k=0;k<t.side.num;k++)
			v[k] = vertex[t.side[k].vertex].pos;

		// bounding box tests
		bool smaller = true;
		for (int k=0;k<t.side.num;k++)
			if (v[k].x >= p.x)
				smaller = false;
		if (smaller)
			continue;

		smaller = true;
		for (int k=1;k<t.side.num;k++){
			if ((v[0].y < p.y) !=  (v[k].y < p.y))
				smaller = false;
			if ((v[0].z < p.z) !=  (v[k].z < p.z))
				smaller = false;
		}
		if (smaller)
			continue;

		// real intersection
		vector col;
		if (t.triangulation_dirty)
			t.update_triangulation(vertex);
		for (int k=t.side.num-2;k>=0;k--)
			if (LineIntersectsTriangle(v[t.side[k].triangulation[0]], v[t.side[k].triangulation[1]], v[t.side[k].triangulation[2]], p, p + vector::EX, col, false))
				if (col.x > p.x)
					n ++;
	}

	// even or odd?
	return ((n % 2) == 1);
}

void Geometry::invert()
{
	for (ModelPolygon &p: polygon)
		p.invert();
}

void Geometry::remove_unused_vertices()
{
	for (ModelVertex &v: vertex)
		v.ref_count = 0;
	for (ModelPolygon &p: polygon)
		for (int i=0;i<p.side.num;i++)
			vertex[p.side[i].vertex].ref_count ++;
	foreachib(ModelVertex &v, vertex, vi)
		if (v.ref_count == 0){
			vertex.erase(vi);
			// correct vertex indices
			for (ModelPolygon &p: polygon)
				for (int i=0;i<p.side.num;i++)
					if (p.side[i].vertex > vi)
						p.side[i].vertex --;
		}
}

bool Geometry::is_mouse_over(MultiView::Window *win, const matrix &mat, vector &tp)
{
	for (ModelPolygon &p: polygon){
		// care for the sense of rotation?
		if (vector::dot(p.temp_normal, win->get_direction()) > 0)
			continue;

		// project all points
		Array<vector> v;
		bool out = false;
		for (int k=0;k<p.side.num;k++){
			vector pp = win->project(mat * vertex[p.side[k].vertex].pos);
			if ((pp.z <= 0) or (pp.z >= 1)){
				out = true;
				break;
			}
			v.add(pp);
		}
		if (out)
			continue;

		// test all sub-triangles
		p.update_triangulation(vertex);
		vector M = win->multi_view->m;
		for (int k=p.side.num-3; k>=0; k--){
			int a = p.side[k].triangulation[0];
			int b = p.side[k].triangulation[1];
			int c = p.side[k].triangulation[2];
			float f,g;
			GetBaryCentric(M, v[a], v[b], v[c], f, g);
			// cursor in triangle?
			if ((f>0) and (g>0) and (f+g<1)){
				vector va = vertex[p.side[a].vertex].pos;
				vector vb = vertex[p.side[b].vertex].pos;
				vector vc = vertex[p.side[c].vertex].pos;
				tp = mat * (va + f*(vb-va) + g*(vc-va));
				return true;
			}
		}
	}
	return false;
}
