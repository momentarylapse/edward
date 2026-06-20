/*
 * Mesh.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "Mesh.h"
#include "Polygon.h"
#include "MeshEdit.h"
#if __has_include(<lib/ygraphics/graphics-fwd.h>)
#include <lib/ygraphics/graphics-impl.h>
#endif
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>
#include <lib/math/plane.h>
#include <lib/math/Box.h>
#include "SkinGenerator.h"

namespace polymesh {

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



Vertex::Vertex(const vec3 &_pos) {
	pos = _pos;
	ref_count = 0;
	normal_mode = 3;//NORMAL_MODE_ANGULAR;
	bone_index = {-1,-1,-1,-1};
	bone_weight = {1,0,0,0};
	normal_dirty = false;
}

Vertex::Vertex() : Vertex(v_0) {}

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




void Mesh::clear() {
	polygons.clear();
	vertices.clear();
	spheres.clear();
	cylinders.clear();
}

bool Mesh::is_empty() const {
	return vertices.num == 0;
}


void Mesh::add_vertex(const vec3 &pos) {
	vertices.add(Vertex(pos));
}

void Mesh::add_polygon(const Array<int> &v, const Array<vec3> &sv) {
	Polygon p;
	p.side.resize(v.num);
	for (int k=0; k<v.num; k++) {
		p.side[k].vertex = v[k];
		p.side[k].uv = sv[k];
	}
	p.material = -1;
	p.normal_dirty = true;
	p.triangulation_dirty = true;
	p.temp_normal = p.get_normal(vertices);
	for (int k=0;k<p.side.num;k++)
		p.side[k].normal = p.temp_normal;
	polygons.add(p);
}

void Mesh::add_polygon_auto_texture(const Array<int> &v) {
	SkinGenerator sg;
	sg.init_point_cloud_boundary(vertices, v);

	Array<vec3> sv;
	for (int k=0; k<v.num; k++)
		sv.add(sg.get(vertices[v[k]].pos));

	add_polygon(v, sv);
}

void Mesh::add_polygon_single_texture(const Array<int> &v, const Array<vec3> &sv) {
	add_polygon(v, sv);
}

void Mesh::add_bezier3(const Array<vec3> &v, int num_x, int num_y, float epsilon)
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

void Mesh::add_easy(int nv, const Array<int> &delta) {
	Array<int> v;
	for (int d: delta)
		v.add(nv + d);
	add_polygon_auto_texture(v);
}

void Mesh::add(const Mesh& geo) {
	int nv = vertices.num;
	int np = polygons.num;
	vertices.append(geo.vertices);
	polygons.append(geo.polygons);
	for (int i=np; i<polygons.num; i++)
		for (int k=0; k<polygons[i].side.num; k++)
			polygons[i].side[k].vertex += nv;
}

void Mesh::weld(float epsilon)
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

void Mesh::weld(const Mesh &geo, float epsilon)
{
}

void Mesh::smoothen()
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

Mesh Mesh::transform(const mat4 &mat) const {
	Mesh mesh = *this;
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

MeshEdit Mesh::edit_inplace(const MeshEdit& edit) {
	return edit.apply_inplace(*this);
}

Mesh Mesh::edit(const MeshEdit& edit, MeshEdit* inv) const {
	return edit.apply(*this, inv);
}


base::set<Edge> Mesh::edges() const {
	base::set<Edge> _edges;
	for (const auto& p: polygons)
		for (int k=0; k<p.side.num; k++) {
			int a = p.side[k].vertex;
			int b = p.side[(k+1)%p.side.num].vertex;
			_edges.add(Edge{min(a, b), max(a, b)});
		}
	return _edges;
}

Array<PolygonCorner> Mesh::get_polygons_around_vertex(int index) const {
	Array<PolygonCorner> corners;

	// unsorted
	for (const auto& p: polygons)
		for (int k=0; k<p.side.num; k++)
			if (p.side[k].vertex == index)
				corners.add({&p, k});

	return corners;
}

int Mesh::next_edge_at_vertex(int index0, int index1) const {
	for (const auto& p: polygons)
		for (int k=0; k<p.side.num; k++)
			if (p.side[k].vertex == index0)
				if (p.side[(k+p.side.num-1)%p.side.num].vertex == index1)
					return p.side[(k+1)%p.side.num].vertex;
	return -1;
}


void Mesh::build_x(DynamicArray& buf) const {
	for (auto &p: const_cast<Array<Polygon>&>(polygons)){
		p.triangulation_dirty = true;
		p.add_to_vertex_buffer(vertices, buf);
	}
}


#if __has_include(<lib/ygraphics/graphics-fwd.h>)
void Mesh::build(ygfx::VertexBuffer *vb) const {
	Array<ygfx::Vertex1> buf;
	build_x(buf);
	vb->update(buf);
}
#endif

bool Mesh::is_closed() const {
	// TOSO
	return false;
	/*for (auto &e: edge)
		if (e.ref_count != 2)
			return false;
	return true;*/
}

bool Mesh::is_inside(const vec3 &p) const {
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

Mesh Mesh::invert() const {
	Mesh mesh = *this;
	for (auto &p: mesh.polygons)
		p.invert();
	return mesh;
}

void Mesh::remove_unused_vertices() {
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

base::optional<Box> Mesh::bounding_box() const {
	if (vertices.num == 0)
		return base::None;
	Box box = {vertices[0].pos, vertices[0].pos};

	for (const auto &v: vertices)
		box = box or Box{v.pos, v.pos};

	for (const auto &b: spheres)
		box = box or Box{
			vertices[b.index].pos - vec3(1,1,1) * b.radius,
			vertices[b.index].pos + vec3(1,1,1) * b.radius};
	return box;
}


void Mesh::update_normals() {
#define NEW_NORMALS 1

#if NEW_NORMALS


	// "flat" triangle normals
	for (auto &t: polygons)
		if (t.normal_dirty) {
			t.normal_dirty = false;
			t.temp_normal = t.get_normal(vertices);
			for (int k=0; k<t.side.num; k++)
				t.side[k].normal = t.temp_normal;
		}

	Array<int> cur_polys;
	Array<int> cur_faces;
	Array<int> cur_groups;
	base::set<int> cur_groups_done;
	for (int v=0; v<vertices.num; v++) {
		cur_polys.clear();
		cur_faces.clear();
		cur_groups.clear();
		cur_groups_done.clear();
		foreachi(auto &p, polygons, i) {
			if (p.smooth_group < 0)
				continue;
			for (int k=0; k<p.side.num; k++) {
				if (p.side[k].vertex == v) {
					cur_polys.add(i);
					cur_faces.add(k);
					cur_groups.add(p.smooth_group);
				}
			}
		}
		for (int i=0; i<cur_groups.num; i++) {
			int g = cur_groups[i];
			if (cur_groups_done.contains(g))
				continue;
			vec3 n = v_0;
			for (int k=i; k<cur_groups.num; k++)
				if (cur_groups[k] == g)
					n += polygons[cur_polys[k]].temp_normal;
			n.normalize();
			for (int k=i; k<cur_groups.num; k++)
				if (cur_groups[k] == g)
					polygons[cur_polys[k]].side[cur_faces[k]].normal = n;
			cur_groups_done.add(g);
		}
	}

#else


	base::set<int> ee, vert;

	// "flat" triangle normals
	for (ModelPolygon &t: polygon)
		if (t.normal_dirty) {
			t.normal_dirty = false;
			t.temp_normal = t.get_normal(vertex);

			for (int k=0;k<t.side.num;k++) {
				t.side[k].normal = t.temp_normal;
				int e = t.side[k].edge;
				if (edge[e].ref_count == 2)
					ee.add(e);
			}
		}

	// round edges?
	for (int ip: ee) {
		ModelEdge &e = edge[ip];

		// adjoined triangles
		ModelPolygon &t1 = polygon[e.polygon[0]];
		ModelPolygon &t2 = polygon[e.polygon[1]];

		ModelVertex &v1 = vertex[e.vertex[0]];
		ModelVertex &v2 = vertex[e.vertex[1]];

		// round?
		e.is_round = false;
		if ((v1.normal_mode == NORMAL_MODE_ANGULAR) || (v2.normal_mode == NORMAL_MODE_ANGULAR))
			e.is_round = (t1.temp_normal * t2.temp_normal > 0.6f);

		if (((v1.normal_mode == NORMAL_MODE_ANGULAR) && (e.is_round)) || (v1.normal_mode == NORMAL_MODE_SMOOTH))
			vert.add(e.vertex[0]);
		if (((v2.normal_mode == NORMAL_MODE_ANGULAR) && (e.is_round)) || (v2.normal_mode == NORMAL_MODE_SMOOTH))
			vert.add(e.vertex[1]);

		/*if (e.IsRound){
			vector n = t1.TempNormal + t2.TempNormal;
			VecNormalize(n);
			for (int k=0;k<3;k++)
				if ((t1.Vertex[k] == e.Vertex[0]) || (t1.Vertex[k] == e.Vertex[1]))
					t1.Normal[k] = n;
			for (int k=0;k<3;k++)
				if ((t2.Vertex[k] == e.Vertex[0]) || (t2.Vertex[k] == e.Vertex[1]))
					t2.Normal[k] = n;
		}*/
	}

	// find all triangles shared by each found vertex
	Array<Array<PolySideData> > poly_side;
	poly_side.resize(vert.num);
	foreachi(ModelPolygon &t, polygon, i){
		for (int k=0;k<t.side.num;k++){
			int n = vert.find(t.side[k].vertex);
			if (n >= 0){
				t.side[k].normal = t.temp_normal;
				PolySideData d;
				d.poly = i;
				d.side = k;
				poly_side[n].add(d);
			}
		}
	}

	// per vertex...
	foreachi(int ip, vert, nn){

		// hard vertex -> nothing to do
		if (vertex[ip].normal_mode == NORMAL_MODE_HARD)
			continue;

		Array<PolySideData> &pd = poly_side[nn];

		// smooth vertex
		if (vertex[ip].normal_mode == NORMAL_MODE_SMOOTH){

			// average normal
			vec3 n = v_0;
			for (int i=0;i<pd.num;i++)
				n += polygon[pd[i].poly].side[pd[i].side].normal;
			n.normalize();
			// apply normal...
			for (int i=0;i<pd.num;i++)
				polygon[pd[i].poly].side[pd[i].side].normal = n;
			continue;
		}

		// angular vertex...

		// find groups of triangles that are connected by round edges
		while (pd.num > 0){

			// start with the 1st triangle
			base::set<int> used;
			used.add(0);

			// search to the right
			bool closed = find_tria_top(this, pd, used, true);

			// search to the left
			if (!closed)
				find_tria_top(this, pd, used, false);

			if (used.num == 1){
				// no smoothly connected triangles...
				pd.erase(0);
				continue;
			}

			// average normal
			vec3 n = v_0;
			for (int i=0;i<used.num;i++)
				n += polygon[pd[used[i]].poly].side[pd[used[i]].side].normal;
			n.normalize();
			// apply normal... and remove from list
			for (int i=used.num-1;i>=0;i--){
				polygon[pd[used[i]].poly].side[pd[used[i]].side].normal = n;
				pd.erase(used[i]);
			}
		}
	}
#endif
}

void geo_poly_find_connected(const Mesh &g, int p0, base::set<int> &polys) {
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

Array<Mesh> Mesh::split_connected() const {
	Array<Mesh> r;
	base::set<int> poly_used;

	for (const auto& [i, p]: enumerate(polygons)) {
		if (poly_used.contains(i))
			continue;

		Mesh g;
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

}

