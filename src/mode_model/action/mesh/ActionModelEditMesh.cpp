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

void* ActionModelEditMesh::execute(Data* d) {
	edit = edit.apply_inplace(*mesh);
	return nullptr;
}

void ActionModelEditMesh::undo(Data* d) {
	execute(d);
}



