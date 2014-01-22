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

	void AddVertex(const vector &pos);
	void AddPolygon(Array<int> &v, Array<vector> &sv);
	void AddPolygonSingleTexture(Array<int> &v, Array<vector> &sv);
	void AddPolygonAutoTexture(Array<int> &v);
	void Add5(int nv, int v0, int v1, int v2, int v3, int v4);
	void Add4(int nv, int v0, int v1, int v2, int v3);
	void Add3(int nv, int v0, int v1, int v2);
	void AddBezier3(Array<vector> &v, int num_x, int num_y, float epsilon);
	void Add(Geometry &geo);

	void Weld(float epsilon);
	void Weld(Geometry &geo, float epsilon);

	void Invert();

	void Transform(const matrix &mat);
	void Smoothen();

	int AddEdge(int a, int b, int tria, int side);
	void UpdateTopology();
	void RemoveUnusedVertices();
	bool IsInside(const vector &v) const;

	void GetBoundingBox(vector &min, vector &max);
	bool IsMouseOver(MultiView::Window *win, vector &tp);

	void Preview(NixVertexBuffer *vb, int num_textures = 1) const;

	Array<ModelVertex> Vertex;
	Array<ModelPolygon> Polygon;
	Array<ModelEdge> Edge;

	bool IsClosed;
};

int GeometrySubtract(Geometry &a, Geometry &b, Geometry &c);
int GeometryAnd(Geometry &a, Geometry &b, Geometry &c);

#endif /* GEOMETRY_H_ */
