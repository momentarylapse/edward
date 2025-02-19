//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELADDPOLYGON_H
#define ACTIONMODELADDPOLYGON_H

#include <action/Action.h>
#include <data/mesh/Polygon.h>
class DataModel;

class ActionModelAddPolygon : public Action {
public:
	explicit ActionModelAddPolygon(const Polygon &p);
	string name() override { return "ModelAddPolygon"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Polygon polygon;
};



#endif //ACTIONMODELADDPOLYGON_H
