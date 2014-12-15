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

class ModelVertex;
class ModelPolygon;
class vector;
class matrix;
class NixVertexBuffer;
namespace MultiView{
	class Window;
};

class Geometry
{
public:
	void clear();

	void addVertex(const vector &pos);
	void addPolygon(Array<int> &v, Array<vector> &sv);
	void addPolygonSingleTexture(Array<int> &v, Array<vector> &sv);
	void addPolygonAutoTexture(Array<int> &v);
	void add5(int nv, int v0, int v1, int v2, int v3, int v4);
	void add4(int nv, int v0, int v1, int v2, int v3);
	void add3(int nv, int v0, int v1, int v2);
	void addBezier3(Array<vector> &v, int num_x, int num_y, float epsilon);
	void add(Geometry &geo);

	void weld(float epsilon);
	void weld(Geometry &geo, float epsilon);

	void invert();

	void transform(const matrix &mat);
	void smoothen();

	int addEdge(int a, int b, int tria, int side);
	void updateTopology();
	void removeUnusedVertices();
	bool isInside(const vector &v) const;

	void getBoundingBox(vector &min, vector &max);
	bool isMouseOver(MultiView::Window *win, vector &tp);

	void preview(NixVertexBuffer *vb, int num_textures = 1) const;

	Array<ModelVertex> vertex;
	Array<ModelPolygon> polygon;
	Array<ModelEdge> edge;

	bool is_closed;
};

int GeometrySubtract(Geometry &a, Geometry &b, Geometry &c);
int GeometryAnd(Geometry &a, Geometry &b, Geometry &c);

#endif /* GEOMETRY_H_ */
