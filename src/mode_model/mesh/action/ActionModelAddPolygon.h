//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELADDPOLYGON_H
#define ACTIONMODELADDPOLYGON_H

#include <action/Action.h>
#include <lib/mesh/Polygon.h>
class DataModel;
struct ModelMesh;

class ActionModelAddPolygon : public Action {
public:
	explicit ActionModelAddPolygon(ModelMesh* mesh, const Polygon &p);
	string name() override { return "ModelAddPolygon"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;
private:
	ModelMesh* mesh;
	Polygon polygon;
};



#endif //ACTIONMODELADDPOLYGON_H
