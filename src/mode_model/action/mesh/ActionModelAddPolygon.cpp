//
// Created by Michael Ankele on 2025-02-19.
//

#include "ActionModelAddPolygon.h"
#include <mode_model/data/DataModel.h>
#include <mode_model/data/ModelMesh.h>

ActionModelAddPolygon::ActionModelAddPolygon(const Polygon& p) {
	polygon = p;
}

void *ActionModelAddPolygon::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	m->mesh->polygons.add(polygon);
	return &m->mesh->polygons.back();
}

void ActionModelAddPolygon::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	m->mesh->polygons.pop();
}
