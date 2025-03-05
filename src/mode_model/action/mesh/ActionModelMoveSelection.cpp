//
// Created by Michael Ankele on 2025-02-20.
//

#include "ActionModelMoveSelection.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/base/iter.h>

ActionModelMoveSelection::ActionModelMoveSelection(ModelMesh* m, const Data::Selection& selection) {
	mesh = m;
	// list of selected objects and save old pos
	if (selection.contains(MultiViewType::MODEL_VERTEX))
		for (const auto& [i, o]: enumerate(mesh->vertices))
			if (selection[MultiViewType::MODEL_VERTEX].contains(i)) {
				index.add(i);
				old_data.add(o.pos);
				type.add(MultiViewType::MODEL_VERTEX);
			}
}

void *ActionModelMoveSelection::execute(Data*) {
	for (const auto& [ii, i]: enumerate(index)) {
		if (type[ii] == MultiViewType::MODEL_VERTEX) {
			mesh->vertices[i].pos = mat * old_data[ii];
		}
	}
	return nullptr;
}

void ActionModelMoveSelection::undo(Data*) {
	for (const auto& [ii, i]: enumerate(index)) {
		if (type[ii] == MultiViewType::MODEL_VERTEX) {
			mesh->vertices[i].pos = old_data[ii];
		}
	}
}
