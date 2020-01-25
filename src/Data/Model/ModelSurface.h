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

class ModelSurface: public MultiView::SingleData {
public:
	ModelSurface();

	Array<ModelPolygon> polygon;
	Array<ModelEdge> edge;
	Set<int> vertex;
	bool is_physical, is_visible;
	bool is_closed;

	DataModel *model;
	SurfaceInsideTestData *inside_data;

	bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data) override;
	bool inRect(MultiView::Window *win, rect &r, void *user_data) override;

	void addVertex(int v);
	void addPolygon(const Array<int> &v, int material, const Array<vector> &sv, int index = -1);
	void removePolygon(int index);
	int addEdgeForNewPolygon(int a, int b, int tria, int side);
	void removeObsoleteEdge(int index);
	void mergeEdges();
	void updateClosed();
	void updateNormals();

	void buildFromPolygons();

	int findEdge(int vertex0, int vertex1);

	bool isInside(const vector &p);
	void beginInsideTests();
	bool insideTest(const vector &p);
	void endInsideTests();
	Array<int> getBoundaryLoop(int v0);
	Array<Array<int> > getConnectedComponents();

	bool testSanity(const string &loc);
};

#endif /* MODELSURFACE_H_ */
