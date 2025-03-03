/*
 * PolygonMesh.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef POLYGONMESH_H_
#define POLYGONMESH_H_

#include "Polygon.h"
#include <multiview/SingleData.h>
#include <lib/base/base.h>
#include <lib/math/vec3.h>
#include <lib/math/vec4.h>
#include <y/graphics-fwd.h>
#include <y/world/Material.h>

class MultiViewWindow;
class VertexStagingBuffer;
class vec3;
class mat4;

/*enum class NormalMode {
	Smooth,
	Angular,
	PerEdge
};*/


struct MeshVertex: multiview::SingleData {
	int normal_mode;
	ivec4 bone_index;
	vec4 bone_weight;

	bool normal_dirty;
	int smoothing_id = -1;
	int ref_count; // polygons

	MeshVertex();
	explicit MeshVertex(const vec3 &pos);
};



struct PolygonMesh {
	void clear();
	bool is_empty() const;

	void add_vertex(const vec3 &pos);
	void add_polygon(const Array<int> &v, const Array<vec3> &sv);
	void add_polygon_single_texture(const Array<int> &v, const Array<vec3> &sv);
	void add_polygon_auto_texture(const Array<int> &v);
	void add_bezier3(const Array<vec3> &v, int num_x, int num_y, float epsilon);
	void add(const PolygonMesh &geo);
	void add_easy(int v0, const Array<int> &delta);

	void weld(float epsilon);
	void weld(const PolygonMesh &geo, float epsilon);

	void invert();

	void transform(const mat4 &mat);
	void smoothen();

	void remove_unused_vertices();
	bool is_inside(const vec3 &v) const;

	void get_bounding_box(vec3 &min, vec3 &max);
	bool is_mouse_over(MultiViewWindow* win, const mat4 &matrix, const vec2& m, vec3 &tp, int& index, bool any_hit);

	void build(VertexBuffer *vb) const;

	Array<MeshVertex> vertices;
	Array<Polygon> polygons;

	bool is_closed() const;

	Array<PolygonMesh> split_connected() const;
	//bool trace(const vec3& p0, const vec3& p1, vec3& tp, int& index);
};

//int GeometrySubtract(const Geometry &a, const Geometry &b, Geometry &c);
//int GeometryAnd(const Geometry &a, const Geometry &b, Geometry &c);

#endif /* POLYGONMESH_H_ */
