/*
 * ModelSurface.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELSURFACE_H_
#define MODELSURFACE_H_

#include "../../MultiView.h"
#include "DataModel.h"
#include "../../lib/base/set.h"
#include "../../lib/types/types.h"

class DataModel;
class ModelSurface;
struct ModelEdge;
struct ModelPolygon;

class ModelSurface: public MultiViewSingleData
{
public:
	ModelSurface();
	virtual ~ModelSurface();

	Array<ModelPolygon> Polygon;
	Array<ModelEdge> Edge;
	Set<int> Vertex;
	bool IsPhysical, IsVisible;
	bool IsClosed;

	DataModel *model;

	void AddVertex(int v);
	bool AddPolygon(Array<int> &v, int material, Array<vector> &sv, int index = -1);
	void RemovePolygon(int index);
	int AddEdgeForNewPolygon(int a, int b, int tria, int side);
	void RemoveObsoleteEdge(int index);
	void MergeEdges();
	void UpdateClosed();
	void UpdateNormals();

	void BuildFromPolygons();


	bool IsInside(const vector &p);
	Array<int> GetBoundaryLoop(int v0);

	bool TestSanity(const string &loc);
};

#endif /* MODELSURFACE_H_ */
