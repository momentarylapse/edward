//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELDELETESELECTION_H
#define ACTIONMODELDELETESELECTION_H


#include <lib/history/Action.h>
#include <lib/mesh/Polygon.h>
#include "../../data/ModelMesh.h"
class DataModel;
struct ModelMesh;

class ActionModelDeleteSelection : public history::Action {
public:
	ActionModelDeleteSelection(ModelMesh* m, const Selection& sel, bool greedy);
	string name() const override { return "ModelDeleteSelection"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:

	int map_vertex(int v) const;
	int unmap_vertex(int v) const;

	ModelMesh* mesh;
	Array<MeshVertex> vertices;
	Array<int> vertex_indices;

	Array<Polygon> polygons;
	Array<int> polygon_indices;

	Array<ModelBall> spheres;
	Array<int> sphere_indices;

	Array<ModelCylinder> cylinders;
	Array<int> cylinder_indices;
};


#endif //ACTIONMODELDELETESELECTION_H
