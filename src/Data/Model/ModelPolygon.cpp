/*
 * ModelPolygon.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */



#include "DataModel.h"
#include "ModelPolygon.h"
#include "../../lib/nix/nix.h"

vector ModelPolygon::getAreaVector(const Array<ModelVertex> &vertex) const
{
	// Newell's method
	vector n = v_0;
	vector p1 = vertex[side.back().vertex].pos;
	for (int i=0; i<side.num; i++){
		vector p0 = p1;
		p1 = vertex[side[i].vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	return n * 0.5f;
}

vector ModelPolygon::getNormal(const Array<ModelVertex> &vertex) const
{
	// Newell's method
	vector n = v_0;
	vector p1 = vertex[side.back().vertex].pos;
	for (int i=0; i<side.num; i++){
		vector p0 = p1;
		p1 = vertex[side[i].vertex].pos;
		n.x += (p0.y - p1.y) * (p0.z + p1.z);
		n.y += (p0.z - p1.z) * (p0.x + p1.x);
		n.z += (p0.x - p1.x) * (p0.y + p1.y);
	}
	n.normalize();
	return n;
}

Array<int> ModelPolygon::getVertices() const
{
	Array<int> v;
	v.resize(side.num);
	for (int i=0; i<side.num; i++)
		v[i] = side[i].vertex;
	return v;
}

Array<vector> ModelPolygon::getSkinVertices() const
{
	Array<vector> sv;
	sv.resize(side.num * MATERIAL_MAX_TEXTURES);
	int n = 0;
	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		for (int i=0; i<side.num; i++)
			sv[n ++] = side[i].skin_vertex[l];
	return sv;
}


static float get_ang(const Array<ModelVertex> &vertex, int a, int b, int c, const vector &flat_n)
{
	vector v1 = vertex[b].pos - vertex[a].pos;
	vector v2 = vertex[c].pos - vertex[b].pos;
	v1.normalize();
	v2.normalize();
	float x = (v1 ^ v2) * flat_n;
	float y = v1 * v2;
	return atan2(x, y);
}

static bool vertex_in_tria(const Array<ModelVertex> &vertex, int a, int b, int c, int v)
{
	float f, g;
	GetBaryCentric(vertex[v].pos, vertex[a].pos, vertex[b].pos, vertex[c].pos, f, g);
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

Array<int> ModelPolygon::triangulate(const Array<ModelVertex> &vertex) const
{
	Array<int> output;

	Array<int> v, vi;
	for (int k=0;k<side.num;k++){
		v.add(side[k].vertex);
		vi.add(k);
	}

	while(v.num > 3){

		// find largest angle (sharpest)
		// TODO: prevent colinear triangles!
		int i_max = 0;
		float f_max = 0;
		for (int i=0;i<v.num;i++){
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

			if (f > f_max){
				// other vertices within this triangle?
				bool ok = true;
				for (int j=0;j<v.num;j++){
					if ((j == i) || (j == ((i+1) % v.num)) || (j == ((i+2) % v.num)))
						continue;
					if (vertex_in_tria(vertex, v[i], v[(i+1) % v.num], v[(i+2) % v.num], v[j])){
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

void ModelPolygon::updateTriangulation(const Array<ModelVertex> &vertex)
{
	Array<int> v = triangulate(vertex);
	for (int i=0; i<v.num; i+=3)
		for (int k=0; k<3; k++)
			side[i/3].triangulation[k] = v[i + k];
	triangulation_dirty = false;
}

void ModelPolygon::addToVertexBuffer(const Array<ModelVertex> &vertex, nix::VertexBuffer *buffer, int num_textures)
{
	if (triangulation_dirty)
		updateTriangulation(vertex);
	for (int i=0; i<side.num-2; i++){
		const ModelPolygonSide &a = side[side[i].triangulation[0]];
		const ModelPolygonSide &b = side[side[i].triangulation[1]];
		const ModelPolygonSide &c = side[side[i].triangulation[2]];
		if (num_textures > 1){
			float ta[MATERIAL_MAX_TEXTURES * 2], tb[MATERIAL_MAX_TEXTURES * 2], tc[MATERIAL_MAX_TEXTURES * 2];
			for (int l=0;l<num_textures;l++){
				ta[l*2  ] = a.skin_vertex[l].x;
				ta[l*2+1] = a.skin_vertex[l].y;
				tb[l*2  ] = b.skin_vertex[l].x;
				tb[l*2+1] = b.skin_vertex[l].y;
				tc[l*2  ] = c.skin_vertex[l].x;
				tc[l*2+1] = c.skin_vertex[l].y;
			}
			buffer->addTriaM(
					vertex[a.vertex].pos, a.normal, ta,
					vertex[b.vertex].pos, b.normal, tb,
					vertex[c.vertex].pos, c.normal, tc);
		}else{
			buffer->addTria(
					vertex[a.vertex].pos, a.normal, a.skin_vertex[0].x, a.skin_vertex[0].y,
					vertex[b.vertex].pos, b.normal, b.skin_vertex[0].x, b.skin_vertex[0].y,
					vertex[c.vertex].pos, c.normal, c.skin_vertex[0].x, c.skin_vertex[0].y);
		}
	}
}

void ModelPolygon::invert()
{
	ModelPolygon pp = *this;
	for (int i=0;i<side.num;i++)
		side[i].vertex = pp.side[side.num - i - 1].vertex;
}
