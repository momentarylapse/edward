/*
 * ModelGeometry.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRY_H_
#define MODELGEOMETRY_H_

#include "../../../lib/base/base.h"

class ModelVertex;
class ModelPolygon;
class vector;

class ModelGeometry
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
	void AddBezier3(Array<vector> &v, int num_x, int num_y);
	void Add(ModelGeometry &geo);

	void Weld(float epsilon);

	void Preview(int vb) const;

	Array<ModelVertex> Vertex;
	Array<ModelPolygon> Polygon;
};

#endif /* MODELGEOMETRY_H_ */
