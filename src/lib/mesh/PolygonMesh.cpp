/*
 * Geometry.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "PolygonMesh.h"
#include "Polygon.h"
#include "MeshEdit.h"
#include "VertexStagingBuffer.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>
#include <lib/math/plane.h>
#include <lib/math/Box.h>
#include "SkinGenerator.h"
#if __has_include(<view/MultiView.h>)
#include <view/MultiView.h>
#endif

class GeometryException : public Exception {
public:
	explicit GeometryException(const string& e) : Exception(e) {}
};

static float Bernstein3(int i, float t) {
	float ti = 1 - t;
	if (i == 0)
		return ti*ti*ti;
	if (i == 1)
		return 3*t*ti*ti;
	if (i == 2)
		return 3*t*t*ti;
	return t*t*t;
}



MeshVertex::MeshVertex(const vec3 &_pos) {
	pos = _pos;
	ref_count = 0;
	normal_mode = 3;//NORMAL_MODE_ANGULAR;
	bone_index = {-1,-1,-1,-1};
	bone_weight = {1,0,0,0};
	normal_dirty = false;
}

MeshVertex::MeshVertex() : MeshVertex(v_0) {}

base::optional<int> Edge::find_other_vertex(int vertex) const {
	if (vertex == index[0])
		return index[1];
	if (vertex == index[1])
		return index[0];
	return base::None;
}


bool Edge::operator==(const Edge& o) {
	return index[0] == o.index[0] and index[1] == o.index[1];
}

bool Edge::operator>(const Edge& o) {
	if (index[0] == o.index[0])
		return index[1] > o.index[1];
	return index[0] > o.index[0];
}




void PolygonMesh::clear() {
	polygons.clear();
	vertices.clear();
	spheres.clear();
	cylinders.clear();
}

bool PolygonMesh::is_empty() const {
	return vertices.num == 0;
}


void PolygonMesh::add_vertex(const vec3 &pos) {
	vertices.add(MeshVertex(pos));
}

void PolygonMesh::add_polygon(const Array<int> &v, const Array<vec3> &sv) {
	Polygon p;
	p.side.resize(v.num);
	for (int k=0; k<v.num; k++) {
		p.side[k].vertex = v[k];
		for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
			p.side[k].skin_vertex[l] = sv[l*v.num + k];
	}
	p.material = -1;
	p.normal_dirty = true;
	p.triangulation_dirty = true;
	p.temp_normal = p.get_normal(vertices);
	for (int k=0;k<p.side.num;k++)
		p.side[k].normal = p.temp_normal;
	polygons.add(p);
}

void PolygonMesh::add_polygon_auto_texture(const Array<int> &v) {
	SkinGenerator sg;
	sg.init_point_cloud_boundary(vertices, v);

	Array<vec3> sv;
	for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv.add(sg.get(vertices[v[k]].pos));

	add_polygon(v, sv);
}

void PolygonMesh::add_polygon_single_texture(const Array<int> &v, const Array<vec3> &sv) {
	Array<vec3> sv2;
	for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
		for (int k=0; k<v.num; k++)
			sv2.add(sv[k]);

	add_polygon(v, sv2);
}

void PolygonMesh::add_bezier3(const Array<vec3> &v, int num_x, int num_y, float epsilon)
{
	vec3 vv[4][4] = {{v[0], v[1], v[2], v[3]}, {v[4], v[5], v[6], v[7]}, {v[8], v[9], v[10], v[11]}, {v[12], v[13], v[14], v[15]}};
	Array<vec3> pp;
	Array<int> vn;
	vn.resize((num_x+1) * (num_y+1));
	bool merged_vertices = false;
	for (int i=0; i<=num_x; i++)
		for (int j=0; j<=num_y; j++){
			float ti = (float)i / (float)num_x;
			float tj = (float)j / (float)num_y;
			vec3 p = v_0;
			for (int k=0; k<=3; k++)
				for (int l=0; l<=3; l++)
					p += Bernstein3(k, ti) * Bernstein3(l, tj) * vv[k][l];
			int old = -1;
			if (epsilon > 0){
				foreachi(vec3 &vv, pp, ii)
					if ((p-vv).length_fuzzy() < epsilon)
						old = vn[ii];
			}
			if (old >= 0){
				merged_vertices = true;
				vn[i*(num_y+1)+j] = old;
			}else{
				vn[i*(num_y+1)+j] = vertices.num;
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
			Array<vec3> sv;
			sv.add(vec3((float) i    / (float)num_y, (float) j    / (float)num_y, 0));
			sv.add(vec3((float) i    / (float)num_y, (float)(j+1) / (float)num_y, 0));
			sv.add(vec3((float)(i+1) / (float)num_y, (float)(j+1) / (float)num_y, 0));
			sv.add(vec3((float)(i+1) / (float)num_y, (float) j    / (float)num_y, 0));
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

void PolygonMesh::add_easy(int nv, const Array<int> &delta) {
	Array<int> v;
	for (int d: delta)
		v.add(nv + d);
	add_polygon_auto_texture(v);
}

void PolygonMesh::add(const PolygonMesh& geo) {
	int nv = vertices.num;
	int np = polygons.num;
	vertices.append(geo.vertices);
	polygons.append(geo.polygons);
	for (int i=np; i<polygons.num; i++)
		for (int k=0; k<polygons[i].side.num; k++)
			polygons[i].side[k].vertex += nv;
}

void PolygonMesh::weld(float epsilon)
{
	//return; // TODO
	//msg_write("------------------------ weld");
	float ep2 = epsilon * epsilon;
	for (int i=vertices.num-2; i>=0; i--)
		for (int j=vertices.num-1; j>i; j--)
			if ((vertices[i].pos - vertices[j].pos).length_sqr() < ep2){
				//msg_write(format("del %d %d", i, j));
				/*bool allowed = true;
				foreach(polygons &p, polygons){
					bool use_i = false;
					bool use_j = false;
					for (int k=0; k<p.Side.num; k++){
						use_i |= (p.Side[k].vertices == i);
						use_j |= (p.Side[k].vertices == j);
					}
					allowed &= (!use_i or !use_j);
				}
				if (!allowed)
					continue;*/

				vertices.erase(j);

				// relink polygons
				for (auto &p: polygons)
					for (int k=0; k<p.side.num; k++){
						if (p.side[k].vertex == j)
							p.side[k].vertex = i;
						else if (p.side[k].vertex > j)
							p.side[k].vertex --;
					}
			}
}

void PolygonMesh::weld(const PolygonMesh &geo, float epsilon)
{
}

void PolygonMesh::smoothen()
{
	Array<vec3> n;
	n.resize(vertices.num);

	// sum all normals (per vertices)
	for (auto &p: polygons)
		for (int k=0;k<p.side.num;k++)
			n[p.side[k].vertex] += p.temp_normal;

	// normalize
	for (int i=0;i<n.num;i++)
		n[i].normalize();

	// apply
	for (auto &p: polygons){
		for (int k=0;k<p.side.num;k++)
			p.side[k].normal = n[p.side[k].vertex];
	}
}

PolygonMesh PolygonMesh::transform(const mat4 &mat) const {
	PolygonMesh mesh = *this;
	for (auto &v: mesh.vertices)
		v.pos = mat * v.pos;
	//matrix mat2 = mat * (float)pow(mat.determinant(), - 1.0f / 3.0f);
	for (auto &p: mesh.polygons){
		/*p.temp_normal = mat2.transform_normal(p.temp_normal);
		for (int k=0;k<p.side.num;k++)
			p.side[k].normal = mat2.transform_normal(p.side[k].normal);*/
		p.temp_normal = p.get_normal(mesh.vertices);
		for (int k=0;k<p.side.num;k++)
			p.side[k].normal = p.temp_normal;
	}
	return mesh;
}

MeshEdit PolygonMesh::edit_inplace(const MeshEdit& edit) {
	return edit.apply_inplace(*this);
}

PolygonMesh PolygonMesh::edit(const MeshEdit& edit, MeshEdit* inv) const {
	return edit.apply(*this, inv);
}


base::set<Edge> PolygonMesh::edges() const {
	base::set<Edge> _edges;
	for (const auto& p: polygons)
		for (int k=0; k<p.side.num; k++) {
			int a = p.side[k].vertex;
			int b = p.side[(k+1)%p.side.num].vertex;
			_edges.add(Edge{min(a, b), max(a, b)});
		}
	return _edges;
}

Array<PolygonCorner> PolygonMesh::get_polygons_around_vertex(int index) const {
	Array<PolygonCorner> corners;

	// unsorted
	for (const auto& p: polygons)
		for (int k=0; k<p.side.num; k++)
			if (p.side[k].vertex == index)
				corners.add({&p, k});

	return corners;
}

int PolygonMesh::next_edge_at_vertex(int index0, int index1) const {
	for (const auto& p: polygons)
		for (int k=0; k<p.side.num; k++)
			if (p.side[k].vertex == index0)
				if (p.side[(k+p.side.num-1)%p.side.num].vertex == index1)
					return p.side[(k+1)%p.side.num].vertex;
	return -1;
}



void PolygonMesh::build(ygfx::VertexBuffer *vb) const {
	VertexStagingBuffer vbs;
#ifdef USING_VULKAN
	int num_textures = 1;
#else
	int num_textures = vb->num_attributes - 2;
#endif
	for (auto &p: const_cast<Array<Polygon>&>(polygons)){
		p.triangulation_dirty = true;
		p.add_to_vertex_buffer(vertices, vbs, num_textures);
	}
	vbs.build(vb, num_textures);
}

bool PolygonMesh::is_closed() const {
	// TOSO
	return false;
	/*for (auto &e: edge)
		if (e.ref_count != 2)
			return false;
	return true;*/
}

bool PolygonMesh::is_inside(const vec3 &p) const {
	// how often does a ray from p intersect the surface?
	int n = 0;
	Array<vec3> v;
	vec3 dir = {1, 0.0001f, 0.00002f};

	for (auto &t: polygons) {

		// plane test
		if ((vec3::dot(p - vertices[t.side[0].vertex].pos, t.temp_normal) > 0) == (vec3::dot(t.temp_normal, dir) > 0))
			continue;

		// polygons data
		if (v.num < t.side.num)
			v.resize(t.side.num);
		for (int k=0;k<t.side.num;k++)
			v[k] = vertices[t.side[k].vertex].pos;

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
		vec3 col;
		if (t.triangulation_dirty)
			t.update_triangulation(vertices);
		for (int k=t.side.num-2;k>=0;k--)
			if (line_intersects_triangle(v[t.side[k].triangulation[0]], v[t.side[k].triangulation[1]], v[t.side[k].triangulation[2]], p, p + dir, col))
				if (col.x > p.x)
					n ++;
	}

	// even or odd?
	return ((n % 2) == 1);
}

PolygonMesh PolygonMesh::invert() const {
	PolygonMesh mesh = *this;
	for (auto &p: mesh.polygons)
		p.invert();
	return mesh;
}

void PolygonMesh::remove_unused_vertices() {
	for (auto &v: vertices)
		v.ref_count = 0;
	for (auto &p: polygons)
		for (int i=0;i<p.side.num;i++)
			vertices[p.side[i].vertex].ref_count ++;
	foreachib(auto &v, vertices, vi)
		if (v.ref_count == 0) {
			vertices.erase(vi);
			// correct vertices indices
			for (auto &p: polygons)
				for (int i=0;i<p.side.num;i++)
					if (p.side[i].vertex > vi)
						p.side[i].vertex --;
		}
}

Box PolygonMesh::bounding_box() const {
	Box box = {v_0, v_0};
	if (vertices.num > 0)
		box = {vertices[0].pos, vertices[0].pos};

	for (const auto &v: vertices)
		box = box or Box{v.pos, v.pos};

	for (const auto &b: spheres)
		box = box or Box{
			vertices[b.index].pos - vec3(1,1,1) * b.radius,
			vertices[b.index].pos + vec3(1,1,1) * b.radius};
	return box;
}

#if __has_include(<view/MultiView.h>)
bool PolygonMesh::is_mouse_over(MultiViewWindow* win, const mat4 &mat, const vec2& m, vec3 &tp, int& index, bool any_hit) {
	vec3 M = vec3(m, 0);
	float zmin = 1;
	for (const auto& [i, p]: enumerate(polygons)) {
		// care for the sense of rotation?
	//	if (vec3::dot(p.temp_normal, win->get_direction()) > 0)
	//		continue;

		// project all points
		Array<vec3> v;
		bool out = false;
		for (int k=0; k<p.side.num; k++) {
			vec3 pp = win->project(mat * vertices[p.side[k].vertex].pos);
			if ((pp.z <= 0) or (pp.z >= 1)){
				out = true;
				break;
			}
			v.add(pp);
		}
		if (out)
			continue;

		// test all sub-triangles
		if (p.triangulation_dirty)
			p.update_triangulation(vertices);
		for (int k=p.side.num-3; k>=0; k--) {
			int a = p.side[k].triangulation[0];
			int b = p.side[k].triangulation[1];
			int c = p.side[k].triangulation[2];
			// FIXME: use 2d bary centric!
			auto fg = bary_centric(M, v[a], v[b], v[c]);
			// cursor in triangle?
			if ((fg.x>0) and (fg.y>0) and (fg.x+fg.y<1)) {
				vec3 va = vertices[p.side[a].vertex].pos;
				vec3 vb = vertices[p.side[b].vertex].pos;
				vec3 vc = vertices[p.side[c].vertex].pos;
				float zz = v[a].z + fg.x*(v[b].z-v[a].z) + fg.x*(v[c].z-v[a].z);
				if (zz < zmin) {
					tp = mat * (va + fg.x*(vb-va) + fg.y*(vc-va));
					index = i;
					zmin = zz;
				}
				if (any_hit)
					return true;
			}
		}
	}
	return zmin < 1;
}
#endif

void geo_poly_find_connected(const PolygonMesh &g, int p0, base::set<int> &polys) {
	base::set<int> verts;
	bool found_more = true;

	auto add_poly = [&verts, &polys, &found_more, &g] (int i) {
		polys.add(i);
		for (auto &f: g.polygons[i].side)
			verts.add(f.vertex);
		found_more = true;
	};
	add_poly(p0);


	auto vertex_overlap = [&verts] (const Polygon &p) {
		for (auto &f: p.side)
			if (verts.contains(f.vertex))
				return true;
		return false;
	};


	while (found_more) {
		found_more = false;
		for (const auto& [i, p]: enumerate(g.polygons)) {
			if (polys.contains(i))
				continue;
			if (vertex_overlap(p))
				add_poly(i);
		}
	}
}

Array<PolygonMesh> PolygonMesh::split_connected() const {
	Array<PolygonMesh> r;
	base::set<int> poly_used;

	for (const auto& [i, p]: enumerate(polygons)) {
		if (poly_used.contains(i))
			continue;

		PolygonMesh g;
		g.vertices = vertices;

		base::set<int> g_polys;
		geo_poly_find_connected(*this, i, g_polys);

		for (int j: g_polys) {
			g.polygons.add(polygons[j]);
			poly_used.add(j);
		}

		g.remove_unused_vertices();
		r.add(g);
	}

	return r;
}

/*bool PolygonMesh::trace(const vec3& p0, const vec3& p1, vec3& tp, int& index) {
	for (const auto& [i, p]: enumerate(polygons)) {
		if (p.triangulation_dirty)
			p.update_triangulation(vertices);

	}
	return false;
}*/

