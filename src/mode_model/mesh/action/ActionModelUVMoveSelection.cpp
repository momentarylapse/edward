#include "ActionModelUVMoveSelection.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/base/iter.h>

ActionModelUVMoveSelection::ActionModelUVMoveSelection(ModelMesh* m, const Selection& selection) {
	mesh = m;
	// list of selected objects and save old pos
	int n0 = 0;
	if (selection.contains(MultiViewType::MODEL_SKIN_VERTEX))
		for (const auto& [ipoly, p]: enumerate(mesh->polygons)) {
			for (int iside=0; iside<p.side.num; iside++)
				if (selection[MultiViewType::MODEL_SKIN_VERTEX].contains(n0 + iside)) {
					index.add(iside);
					polygon_no.add(ipoly);
					old_data.add(p.side[iside].skin_vertex[0]);
				}
			n0 += p.side.num;
		}
}

void *ActionModelUVMoveSelection::execute(history::Data*) {
	for (const auto& [ii, i]: enumerate(index))
		mesh->polygons[polygon_no[ii]].side[i].skin_vertex[0] = mat * old_data[ii];
	return nullptr;
}

void ActionModelUVMoveSelection::undo(history::Data*) {
	for (const auto& [ii, i]: enumerate(index))
		mesh->polygons[polygon_no[ii]].side[i].skin_vertex[0] = old_data[ii];
}
