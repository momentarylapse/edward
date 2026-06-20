/*
 * Mesh.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "Polygon.h"
#include <lib/base/base.h>
#include <lib/base/set.h>
#include <lib/base/optional.h>
#include <lib/math/vec3.h>
#include <lib/math/vec4.h>
#if __has_include(<lib/ygraphics/graphics-fwd.h>)
#include <lib/ygraphics/graphics-fwd.h>
#endif

class MultiViewWindow;
struct vec3;
struct mat4;
struct Box;

namespace polymesh {

struct Vertex {
	vec3 pos;
	int normal_mode;
	ivec4 bone_index;
	vec4 bone_weight;

	bool normal_dirty;
	int smoothing_id = -1;
	int ref_count; // polygons

	Vertex();
	explicit Vertex(const vec3 &pos);
};


struct Sphere {
	int index;
	float radius;
};

struct Cylinder {
	int index[2];
	float radius;
	bool round;
};

struct Edge {
	int index[2]; // sorted
	base::optional<int> find_other_vertex(int vertex) const;
	bool operator==(const Edge& o);
	bool operator>(const Edge& o);
};

struct PolygonCorner {
	const Polygon* polygon;
	int side;
};

struct MeshEdit;


struct Mesh {
	void clear();
	bool is_empty() const;

	base::optional<Box> bounding_box() const;

	void add_vertex(const vec3 &pos);
	void add_polygon(const Array<int> &v, const Array<vec3> &sv);
	void add_polygon_single_texture(const Array<int> &v, const Array<vec3> &sv);
	void add_polygon_auto_texture(const Array<int> &v);
	void add_bezier3(const Array<vec3> &v, int num_x, int num_y, float epsilon);
	void add(const Mesh &geo);
	void add_easy(int v0, const Array<int> &delta);

	void weld(float epsilon);
	void weld(const Mesh &geo, float epsilon);

	Mesh invert() const;
	Mesh transform(const mat4 &mat) const;
	void smoothen();
	MeshEdit edit_inplace(const MeshEdit& edit);
	Mesh edit(const MeshEdit& edit, MeshEdit* inv=nullptr) const;

	base::set<Edge> edges() const;
	Array<PolygonCorner> get_polygons_around_vertex(int index) const;
	int next_edge_at_vertex(int index0, int index1) const;

	void remove_unused_vertices();
	bool is_inside(const vec3 &v) const;

	void update_normals();

	void build_x(DynamicArray& buf) const;
#if __has_include(<lib/ygraphics/graphics-fwd.h>)
	void build(ygfx::VertexBuffer *vb) const;
#endif

	Array<Vertex> vertices;
	Array<Polygon> polygons;

	// purely physical
	Array<Sphere> spheres;
	Array<Cylinder> cylinders;

	bool is_closed() const;

	Array<Mesh> split_connected() const;
	//bool trace(const vec3& p0, const vec3& p1, vec3& tp, int& index);
};

}


