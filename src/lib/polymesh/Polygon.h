//
// Created by Michael Ankele on 2025-02-19.
//

#pragma once

#include <lib/base/base.h>
#include <lib/math/vec3.h>
#include <lib/math/vec4.h>

class MultiViewWindow;
struct vec3;
struct mat4;
struct MeshVertex;
struct Edge;

struct PolygonSide {
	int vertex;
	vec3 uv;
	int normal_index;
	int smoothing_id = -1;
	vec3 normal;
	unsigned char triangulation[3];
};

struct Polygon {
	Array<PolygonSide> side;
	vec3 temp_normal;
	bool normal_dirty = true;
	bool triangulation_dirty = true;
	int material = 0;
	int smooth_group = -1;

	Array<int> triangulate(const Array<MeshVertex> &vertex) const;
	void update_triangulation(const Array<MeshVertex> &vertex);
	vec3 get_normal(const Array<MeshVertex> &vertex) const;
	vec3 get_area_vector(const Array<MeshVertex> &vertex) const;
	Array<int> get_vertices() const;
	Array<Edge> get_edges() const;
	Array<vec3> get_uvs() const;
	int next_vertex(int index) const;
	int previous_vertex(int index) const;
	Edge get_side_edge_in(int side_no) const;
	Edge get_side_edge_out(int side_no) const;
	void invert();

	void add_to_vertex_buffer(const Array<MeshVertex> &vertex, DynamicArray& buf);
};


