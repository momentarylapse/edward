//
// Created by michi on 10/30/25.
//

#include "ActionModelAlignToGrid.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/base/iter.h>

ActionModelAlignToGrid::ActionModelAlignToGrid(ModelMesh* m, const Data::Selection& selection, const std::function<vec3(const vec3&)>& f) {
	mesh = m;
	// list of selected objects and save old pos
	if (selection.contains(MultiViewType::MODEL_VERTEX))
		for (const auto& [i, o]: enumerate(mesh->vertices))
			if (selection[MultiViewType::MODEL_VERTEX].contains(i)) {
				index.add(i);
				old_pos.add(f(o.pos));
			}
}

void *ActionModelAlignToGrid::execute(Data*) {
	for (const auto& [ii, i]: enumerate(index)) {
		std::swap(mesh->vertices[i].pos, old_pos[ii]);
	}
	return nullptr;
}

void ActionModelAlignToGrid::undo(Data* d) {
	execute(d);
}
