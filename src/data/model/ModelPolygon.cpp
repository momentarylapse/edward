/*
 * ModelPolygon.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */



#include "DataModel.h"
#include "ModelPolygon.h"
#include "ModelMesh.h" // for Vertex
#include "../../lib/nix/nix.h"
#include "../../lib/math/vec2.h"


void VertexStagingBuffer::build(nix::VertexBuffer *vb, int num_textures) {
	Array<float> temp;
	int d = 3 + 3 + 2*num_textures;
	temp.resize(p.num * d);
	for (int i=0; i<p.num; i++) {
		*(vec3*)&temp[i * d] = p[i];
		*(vec3*)&temp[i * d + 3] = n[i];
		for (int l=0; l<num_textures; l++)
			*(vec2*)&temp[i * d + 6 + l*2] = *(vec2*)&uv[l][2*i];
	}
	vb->update(temp);
	/*vb->update(0, p);
	vb->update(1, n);
	for (int l=0; l<num_textures; l++)
		vb->update(l+2, uv[l]);*/

}

vec3 ModelPolygon::get_area_vector(const Array<ModelVertex> &vertex) const {
	// Newell's method
	vec3 n = v_0;
	vec3 p1 = vertex[side.back().vertex].pos;
	for (int i=0; i<side.num; i++) {
		vec3 p0 = p1;
		p1 = vertex[side[i].vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	return n * 0.5f;
}

vec3 ModelPolygon::get_normal(const Array<ModelVertex> &vertex) const {
	// Newell's method
	vec3 n = v_0;
	vec3 p1 = vertex[side.back().vertex].pos;
	for (int i=0; i<side.num; i++) {
		vec3 p0 = p1;
		p1 = vertex[side[i].vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	n.normalize();
	return n;
}

Array<int> ModelPolygon::get_vertices() const {
	Array<int> v;
	v.resize(side.num);
	for (int i=0; i<side.num; i++)
		v[i] = side[i].vertex;
	return v;
}

Array<vec3> ModelPolygon::get_skin_vertices() const {
	Array<vec3> sv;
	sv.resize(side.num * MATERIAL_MAX_TEXTURES);
	int n = 0;
	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		for (int i=0; i<side.num; i++)
			sv[n ++] = side[i].skin_vertex[l];
	return sv;
}


static float get_ang(const Array<ModelVertex> &vertex, int a, int b, int c, const vec3 &flat_n) {
	vec3 v1 = vertex[b].pos - vertex[a].pos;
	vec3 v2 = vertex[c].pos - vertex[b].pos;
	v1.normalize();
	v2.normalize();
	float x = vec3::dot(vec3::cross(v1, v2), flat_n);
	float y = vec3::dot(v1, v2);
	return atan2(x, y);
}

static bool vertex_in_tria(const Array<ModelVertex> &vertex, int a, int b, int c, int v) {
	auto fg = bary_centric(vertex[v].pos, vertex[a].pos, vertex[b].pos, vertex[c].pos);
	return ((fg.x > 0) and (fg.y > 0) and (fg.x + fg.y < 1));
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

Array<int> ModelPolygon::triangulate(const Array<ModelVertex> &vertex) const {
	Array<int> output;

	Array<int> v, vi;
	for (int k=0; k<side.num; k++) {
		v.add(side[k].vertex);
		vi.add(k);
	}

	while(v.num > 3) {

		// find largest angle (sharpest)
		// TODO: prevent colinear triangles!
		int i_max = 0;
		float f_max = 0;
		for (int i=0;i<v.num;i++) {
			float f = get_ang(vertex, v[i], v[(i+1) % v.num], v[(i+2) % v.num], temp_normal);
			if (f < 0)
				continue;
			// cheat: ...
			float f_n = get_ang(vertex, v[(i+1) % v.num], v[(i+2) % v.num], v[(i+3) % v.num], temp_normal);
			float f_l = get_ang(vertex, v[(i-1+v.num) % v.num], v[i], v[(i+1) % v.num], temp_normal);
			if (f_n >= 0)
				f += 0.01f / (f_n + 0.01f);
			if (f_l >= 0)
				f += 0.01f / (f_l + 0.01f);

			if (f > f_max) {
				// other vertices within this triangle?
				bool ok = true;
				for (int j=0;j<v.num;j++) {
					if ((j == i) or (j == ((i+1) % v.num)) or (j == ((i+2) % v.num)))
						continue;
					if (vertex_in_tria(vertex, v[i], v[(i+1) % v.num], v[(i+2) % v.num], v[j])) {
						ok = false;
						break;
					}
				}

				if (ok) {
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

void ModelPolygon::update_triangulation(const Array<ModelVertex> &vertex) {
	auto v = triangulate(vertex);
	for (int i=0; i<v.num; i+=3)
		for (int k=0; k<3; k++)
			side[i/3].triangulation[k] = v[i + k];
	triangulation_dirty = false;
}

void ModelPolygon::add_to_vertex_buffer(const Array<ModelVertex> &vertex, VertexStagingBuffer &vbs, int num_textures) {
	if (triangulation_dirty)
		update_triangulation(vertex);
	for (int i=0; i<side.num-2; i++) {
		auto &a = side[side[i].triangulation[0]];
		auto &b = side[side[i].triangulation[1]];
		auto &c = side[side[i].triangulation[2]];
		vbs.p.add(vertex[a.vertex].pos);
		vbs.p.add(vertex[b.vertex].pos);
		vbs.p.add(vertex[c.vertex].pos);
		vbs.n.add(a.normal);
		vbs.n.add(b.normal);
		vbs.n.add(c.normal);
		for (int l=0; l<num_textures; l++){
			vbs.uv[l].add(a.skin_vertex[l].x);
			vbs.uv[l].add(a.skin_vertex[l].y);
			vbs.uv[l].add(b.skin_vertex[l].x);
			vbs.uv[l].add(b.skin_vertex[l].y);
			vbs.uv[l].add(c.skin_vertex[l].x);
			vbs.uv[l].add(c.skin_vertex[l].y);
		}
	}
}

void ModelPolygon::invert() {
	ModelPolygon pp = *this;
	for (int i=0; i<side.num; i++) {
		side[i].vertex = pp.side[side.num - i - 1].vertex;
		memcpy(side[i].skin_vertex, pp.side[side.num - i - 1].skin_vertex, sizeof(side[i].skin_vertex));
		side[i].normal = - pp.side[side.num - i - 1].normal;
	}
	temp_normal = - temp_normal;
}
