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

class ModelVertex;
class ModelPolygon;
class vector;
class matrix;
namespace nix {
	class VertexBuffer;
};
namespace MultiView {
	class Window;
};

class Geometry {
public:
	void clear();

	void add_vertex(const vector &pos);
	void add_polygon(const Array<int> &v, const Array<vector> &sv);
	void add_polygon_single_texture(const Array<int> &v, const Array<vector> &sv);
	void add_polygon_auto_texture(const Array<int> &v);
	void add_bezier3(const Array<vector> &v, int num_x, int num_y, float epsilon);
	void add(const Geometry &geo);
	void add_easy(int v0, const Array<int> &delta);

	void weld(float epsilon);
	void weld(const Geometry &geo, float epsilon);

	void invert();

	void transform(const matrix &mat);
	void smoothen();

	int add_edge(int a, int b, int tria, int side);
	void update_topology();
	void remove_unused_vertices();
	bool is_inside(const vector &v) const;

	void get_bounding_box(vector &min, vector &max);
	bool is_mouse_over(MultiView::Window *win, const matrix &mat, vector &tp);

	void build(nix::VertexBuffer *vb) const;

	Array<ModelVertex> vertex;
	Array<ModelPolygon> polygon;
	Array<ModelEdge> edge;

	bool is_closed;
};

int GeometrySubtract(Geometry &a, Geometry &b, Geometry &c);
int GeometryAnd(Geometry &a, Geometry &b, Geometry &c);

#endif /* GEOMETRY_H_ */
