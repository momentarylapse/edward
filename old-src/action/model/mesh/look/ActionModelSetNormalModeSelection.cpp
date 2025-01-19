/*
 * ActionModelSetNormalModeSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetNormalModeSelection.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"

ActionModelSetNormalModeSelection::ActionModelSetNormalModeSelection(DataModel *m, int _mode) {
	mode = _mode;
	foreachi(auto &v, m->mesh->vertex, i)
		if (v.is_selected)
			index.add(i);
	foreachi(auto &p, m->mesh->polygon, i)
		if (p.is_selected)
			p_index.add(i);
	smooth_group = -1;
	if (mode == NORMAL_MODE_SMOOTH)
		smooth_group = randi(50000);
}

void *ActionModelSetNormalModeSelection::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);

	// per vertex
	old_mode.clear();
	for (int i: index) {
		old_mode.add(m->mesh->vertex[i].normal_mode);
		m->mesh->vertex[i].normal_mode = mode;
	}

	// per vertex
	old_group.clear();
	for (int i: p_index) {
		old_group.add(m->mesh->polygon[i].smooth_group);
		m->mesh->polygon[i].smooth_group = smooth_group;
	}
	m->set_normals_dirty_by_vertices(index);

	return nullptr;
}



void ActionModelSetNormalModeSelection::undo(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);

	// per vertex
	foreachi(int i, index, ii)
		m->mesh->vertex[i].normal_mode = old_mode[ii];
	foreachi (int i, p_index, ii)
		m->mesh->polygon[i].smooth_group = old_group[ii];

	m->set_normals_dirty_by_vertices(index);
}


