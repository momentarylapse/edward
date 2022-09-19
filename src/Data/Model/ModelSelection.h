/*
 * ModelSelection.h
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MODEL_MODELSELECTION_H_
#define SRC_DATA_MODEL_MODELSELECTION_H_

#include "../../lib/base/base.h"
#include "../../lib/base/set.h"

class ModelMesh;
class ModelEdge;

class ModelSelection {
public:
	base::set<int> vertex;
	base::set<int> polygon;

	struct Edge {
		int a, b;
	};
	Array<Edge> edge;
	void set(const ModelEdge &e);
	bool has(const ModelEdge &e) const;

	base::set<int> bone;
	base::set<int> cylinder;
	base::set<int> ball;

	void clear();
	void expand_to_surfaces(ModelMesh *m);
	bool consistent_surfaces(ModelMesh *m) const;

	ModelSelection filter_view_stage(ModelMesh *m, int view_stage) const;

	static ModelSelection all(ModelMesh *m);
};

#endif /* SRC_DATA_MODEL_MODELSELECTION_H_ */
