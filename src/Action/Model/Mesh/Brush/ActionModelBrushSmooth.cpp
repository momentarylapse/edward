/*
 * ActionModelBrushSmooth.cpp
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#include "ActionModelBrushSmooth.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"

ActionModelBrushSmooth::ActionModelBrushSmooth(const vector &_pos, const vector &_n, float _radius) {
	pos = _pos;
	n = _n;
	radius = _radius;
}

void* ActionModelBrushSmooth::execute(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	float r2 = radius * radius;

	for (int i=0;i<m->edit_mesh->vertex.num;i++) {
		float d2 = (pos - m->edit_mesh->vertex[i].pos).length_sqr();
		if (d2 < r2 * 2) {
			index.add(i);
			pos_old.add(m->edit_mesh->vertex[i].pos);
			vector d = (m->edit_mesh->vertex[i].pos - pos);
			d = d - vector::dot(d, n) * n * exp(- d2 / r2 * 2);
			m->edit_mesh->vertex[i].pos = pos + d;
		}
	}
	m->set_normals_dirty_by_vertices(index);


	return nullptr;
}

void ActionModelBrushSmooth::undo(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->set_normals_dirty_by_vertices(index);
	foreachi(int i, index, ii)
		m->edit_mesh->vertex[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
