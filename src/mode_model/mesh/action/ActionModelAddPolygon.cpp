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

void *ActionModelAddPolygon::execute(Data* data) {
	auto d = dynamic_cast<DataModel*>(data);
	mesh->polygons.add(polygon);
	d->out_topology_changed();
	return &mesh->polygons.back();
}

void ActionModelAddPolygon::undo(Data* data) {
	auto d = dynamic_cast<DataModel*>(data);
	mesh->polygons.pop();
	d->out_topology_changed();
}
