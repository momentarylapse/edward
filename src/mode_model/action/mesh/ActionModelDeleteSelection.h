//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELDELETESELECTION_H
#define ACTIONMODELDELETESELECTION_H


#include <action/Action.h>
#include <data/mesh/Polygon.h>
class DataModel;

class ActionModelDeleteSelection : public Action {
public:
	ActionModelDeleteSelection(DataModel* m, const Data::Selection& sel, bool greedy);
	string name() override { return "ModelDeleteSelection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:

	int map_vertex(int v) const;
	int unmap_vertex(int v) const;

	Array<MeshVertex> vertices;
	Array<int> vertex_indices;

	Array<Polygon> polygons;
	Array<int> polygon_indices;
};


#endif //ACTIONMODELDELETESELECTION_H
