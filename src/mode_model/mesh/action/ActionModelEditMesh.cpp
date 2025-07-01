//
// Created by Michael Ankele on 2025-05-18.
//

#include "ActionModelEditMesh.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"

ActionModelEditMesh::ActionModelEditMesh(ModelMesh* _mesh, const MeshEdit& _edit) {
	mesh = _mesh;
	edit = _edit;
}

void* ActionModelEditMesh::execute(Data* data) {
	auto d = dynamic_cast<DataModel*>(data);
	edit = edit.apply_inplace(*mesh);

	d->out_topology_changed();
	return nullptr;
}

void ActionModelEditMesh::undo(Data* data) {
	execute(data);
}



