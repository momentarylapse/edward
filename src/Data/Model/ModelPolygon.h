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

#include "../../x/material.h"

class ModelPolygonSide
{
public:
	int Vertex;
	int Edge;
	int EdgeDirection; // which no of triangle in edge's list are we?
	vector SkinVertex[MATERIAL_MAX_TEXTURES];
	int NormalIndex;
	vector Normal;
	unsigned char Triangulation[3];
};

class ModelPolygon: public MultiViewSingleData
{
public:
	Array<ModelPolygonSide> Side;
	vector TempNormal;
	bool NormalDirty, TriangulationDirty;
	int Material;

	Array<int> Triangulate(const Array<ModelVertex> &vertex) const;
	void UpdateTriangulation(const Array<ModelVertex> &vertex);
	vector GetNormal(const Array<ModelVertex> &vertex) const;
	vector GetAreaVector(const Array<ModelVertex> &vertex) const;
	Array<int> GetVertices() const;
	Array<vector> GetSkinVertices() const;
	void Invert();
	void AddToVertexBuffer(const Array<ModelVertex> &vertex, int buffer, int num_textures);
};


#endif /* MODELPOLYGON_H_ */
