/*
 * ActionModelPasteMesh.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionModelPasteMesh.h"
#include "../../../data/DataModel.h"
#include "../../../data/ModelMesh.h"

ActionModelPasteMesh::ActionModelPasteMesh(ModelMesh* m, const polymesh::Mesh& geo, int default_material) {
	mesh = m;

	Array<int> vindex;
	for (const auto& v: geo.vertices)
		vindex.add(edit.add_vertex(v));

	for (auto p: geo.polygons) {
		for (auto& s: p.side)
			s.vertex = vindex[s.vertex];
		if (p.material < 0)
			p.material = default_material;
		edit.add_polygon(p);
	}
}

void *ActionModelPasteMesh::execute(history::Data* data) {
	auto d = dynamic_cast<DataModel*>(data);
	edit = edit.apply_inplace(*mesh);

	d->out_mesh_edited(edit);
	return nullptr;
}

void ActionModelPasteMesh::undo(history::Data* data) {
	execute(data);
}
