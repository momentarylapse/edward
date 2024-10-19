/*
 * Geometry.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include "../DataModel.h"
#include "../ModelPolygon.h"
#include "../ModelMesh.h"
#include <y/graphics-fwd.h>

class ModelVertex;
class ModelPolygon;
class vec3;
class mat4;
namespace MultiView {
	class Window;
};

class Geometry {
public:
	void clear();

	void add_vertex(const vec3 &pos);
	void add_polygon(const Array<int> &v, const Array<vec3> &sv);
	void add_polygon_single_texture(const Array<int> &v, const Array<vec3> &sv);
	void add_polygon_auto_texture(const Array<int> &v);
	void add_bezier3(const Array<vec3> &v, int num_x, int num_y, float epsilon);
	void add(const Geometry &geo);
	void add_easy(int v0, const Array<int> &delta);

	void weld(float epsilon);
	void weld(const Geometry &geo, float epsilon);

	void invert();

	void transform(const mat4 &mat);
	void smoothen();

	int add_edge(int a, int b, int tria, int side) const;
	void update_topology() const;
	void remove_unused_vertices();
	bool is_inside(const vec3 &v) const;

	void get_bounding_box(vec3 &min, vec3 &max);
	bool is_mouse_over(MultiView::Window *win, const mat4 &mat, vec3 &tp);

	void build(VertexBuffer *vb) const;

	Array<ModelVertex> vertex;
	Array<ModelPolygon> polygon;
	mutable Array<ModelEdge> edge;

	bool is_closed() const;

	Array<Geometry> split_connected() const;
};

int GeometrySubtract(const Geometry &a, const Geometry &b, Geometry &c);
int GeometryAnd(const Geometry &a, const Geometry &b, Geometry &c);

#endif /* GEOMETRY_H_ */
