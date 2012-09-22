/*
 * ModeModelSurface.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELSURFACE_H_
#define MODEMODELSURFACE_H_

#include "../../MultiView.h"
#include "DataModel.h"
#include "../../lib/base/set.h"
#include "../../lib/types/types.h"

class DataModel;
class ModelSurface;
struct ModelEdge;
struct ModelTriangle;

class ModelSurface: public MultiViewSingleData
{
public:
	ModelSurface();
	virtual ~ModelSurface();

	Array<ModelTriangle> Triangle;
	Array<ModelEdge> Edge;
	Set<int> Vertex;
	bool IsPhysical, IsVisible;
	bool IsClosed;

	DataModel *model;

	void AddVertex(int v);
	void AddTriangle(int a, int b, int c, int material, const vector *sa, const vector *sb, const vector *sc, int index = -1);
	void RemoveTriangle(int index);
	int AddEdgeForNewTriangle(int a, int b, int tria, int side);
	void RemoveObsoleteEdge(int index);
	void MergeEdges();
	void UpdateClosed();
	void UpdateNormals();

	void BuildFromTriangles();


	bool IsInside(const vector &p);

	void TestSanity(const string &loc);
};

#endif /* MODEMODELSURFACE_H_ */
