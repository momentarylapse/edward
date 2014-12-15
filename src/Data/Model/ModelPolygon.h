/*
 * ModelPolygon.h
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#ifndef MODELPOLYGON_H_
#define MODELPOLYGON_H_

class DataModel;
class ModelVertex;
class NixVertexBuffer;

#include "../../x/material.h"
#include "../../MultiView/SingleData.h"

class ModelPolygonSide
{
public:
	int vertex;
	int edge;
	int edge_direction; // which no of triangle in edge's list are we?
	vector skin_vertex[MATERIAL_MAX_TEXTURES];
	int normal_index;
	vector normal;
	unsigned char triangulation[3];
};

class ModelPolygon: public MultiView::SingleData
{
public:
	Array<ModelPolygonSide> side;
	vector temp_normal;
	bool normal_dirty, triangulation_dirty;
	int material;

	virtual bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(MultiView::Window *win, rect &r, void *user_data);

	Array<int> Triangulate(const Array<ModelVertex> &vertex) const;
	void UpdateTriangulation(const Array<ModelVertex> &vertex);
	vector GetNormal(const Array<ModelVertex> &vertex) const;
	vector GetAreaVector(const Array<ModelVertex> &vertex) const;
	Array<int> GetVertices() const;
	Array<vector> GetSkinVertices() const;
	void Invert();
	void AddToVertexBuffer(const Array<ModelVertex> &vertex, NixVertexBuffer *vb, int num_textures);
};


#endif /* MODELPOLYGON_H_ */
