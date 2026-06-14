//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELDELETESELECTION_H
#define ACTIONMODELDELETESELECTION_H


#include <lib/history/Action.h>
#include <lib/polymesh/Polygon.h>
#include "../../../data/ModelMesh.h"
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
	Array<polymesh::Vertex> vertices;
	Array<int> vertex_indices;

	Array<polymesh::Polygon> polygons;
	Array<int> polygon_indices;

	Array<polymesh::Sphere> spheres;
	Array<int> sphere_indices;

	Array<polymesh::Cylinder> cylinders;
	Array<int> cylinder_indices;
};


#endif //ACTIONMODELDELETESELECTION_H
