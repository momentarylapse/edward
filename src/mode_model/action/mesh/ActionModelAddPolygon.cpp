//
// Created by Michael Ankele on 2025-02-19.
//

#include "ActionModelAddPolygon.h"
#include <mode_model/data/DataModel.h>
#include <mode_model/data/ModelMesh.h>

ActionModelAddPolygon::ActionModelAddPolygon(ModelMesh* _mesh, const Polygon& p) {
	mesh = _mesh;
	polygon = p;
}

void *ActionModelAddPolygon::execute(Data*) {
	mesh->polygons.add(polygon);
	return &mesh->polygons.back();
}

void ActionModelAddPolygon::undo(Data*) {
	mesh->polygons.pop();
}
