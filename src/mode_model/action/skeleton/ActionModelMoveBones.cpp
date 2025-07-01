//
// Created by Michael Ankele on 2025-07-01.
//

#include "ActionModelMoveBones.h"
#include "../../data/DataModel.h"
#include <Session.h>
#include <lib/base/iter.h>

ActionModelMoveBones::ActionModelMoveBones(DataModel* m, const Data::Selection& selection) {
	// list of selected objects and save old pos
	if (selection.contains(MultiViewType::SKELETON_BONE))
		for (const auto& [i, o]: enumerate(m->bones))
			if (selection[MultiViewType::SKELETON_BONE].contains(i)) {
				index.add(i);
				old_data.add(o.pos);
			}
}

void *ActionModelMoveBones::execute(Data* d) {
	auto m = static_cast<DataModel*>(d);
	for (const auto& [ii, i]: enumerate(index)) {
		m->bones[i].pos = mat * old_data[ii];
	}
	return nullptr;
}

void ActionModelMoveBones::undo(Data* d) {
	auto m = static_cast<DataModel*>(d);
	for (const auto& [ii, i]: enumerate(index)) {
		m->bones[i].pos = old_data[ii];
	}
}
