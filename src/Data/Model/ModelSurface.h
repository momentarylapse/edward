/*
 * ModelSurface.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELSURFACE_H_
#define MODELSURFACE_H_

#include "../../MultiView/SingleData.h"
#include "DataModel.h"
#include "../../lib/base/set.h"
#include "../../lib/math/math.h"

class DataModel;
class ModelSurface;
class ModelEdge;
class ModelPolygon;

struct SurfaceInsideTestData;

class ModelSurface: public MultiView::SingleData
{
public:
	ModelSurface();

	Array<ModelPolygon> Polygon;
	Array<ModelEdge> Edge;
	Set<int> Vertex;
	bool IsPhysical, IsVisible;
	bool IsClosed;

	DataModel *model;
	SurfaceInsideTestData *inside_data;

	virtual bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(MultiView::Window *win, rect &r, void *user_data);

	void AddVertex(int v);
	void AddPolygon(Array<int> &v, int material, Array<vector> &sv, int index = -1);
	void RemovePolygon(int index);
	int AddEdgeForNewPolygon(int a, int b, int tria, int side);
	void RemoveObsoleteEdge(int index);
	void MergeEdges();
	void UpdateClosed();
	void UpdateNormals();

	void BuildFromPolygons();

	int FindEdge(int vertex0, int vertex1);

	bool IsInside(const vector &p);
	void BeginInsideTests();
	bool InsideTest(const vector &p);
	void EndInsideTests();
	Array<int> GetBoundaryLoop(int v0);
	Array<Array<int> > GetConnectedComponents();

	bool TestSanity(const string &loc);
};

#endif /* MODELSURFACE_H_ */
