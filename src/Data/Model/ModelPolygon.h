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
};

class ModelPolygon: public MultiViewSingleData
{
public:
	Array<ModelPolygonSide> Side;
	vector TempNormal;
	bool NormalDirty;
	int Material;

	Array<int> Triangulate(DataModel *m) const;
	vector GetNormal(DataModel *m) const;
	Array<int> GetVertices() const;
	Array<vector> GetSkinVertices() const;
};


#endif /* MODELPOLYGON_H_ */
