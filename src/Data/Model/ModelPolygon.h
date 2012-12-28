/*
 * ModelPolygon.h
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#ifndef MODELPOLYGON_H_
#define MODELPOLYGON_H_

class DataModel;

class ModelPolygonSide
{
public:
	int Vertex;
	int Edge;
	int EdgeDirection; // which no of triangle in edge's list are we?
	vector SkinVertex[MODEL_MAX_TEXTURES];
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

	Array<int> Triangulate(const DataModel *m) const;
	void UpdateTriangulation(const DataModel *m);
	vector GetNormal(const DataModel *m) const;
	Array<int> GetVertices() const;
	Array<vector> GetSkinVertices() const;
};


#endif /* MODELPOLYGON_H_ */
