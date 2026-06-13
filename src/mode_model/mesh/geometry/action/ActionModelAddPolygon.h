//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELADDPOLYGON_H
#define ACTIONMODELADDPOLYGON_H

#include <lib/history/Action.h>
#include <lib/polymesh/Polygon.h>
class DataModel;
struct ModelMesh;

class ActionModelAddPolygon : public history::Action {
public:
	explicit ActionModelAddPolygon(ModelMesh* mesh, const Polygon &p);
	string name() const override { return "ModelAddPolygon"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	ModelMesh* mesh;
	Polygon polygon;
};



#endif //ACTIONMODELADDPOLYGON_H
