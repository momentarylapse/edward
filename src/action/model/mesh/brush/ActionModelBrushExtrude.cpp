/*
 * ActionModelBrushExtrude.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ActionModelBrushExtrude.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"

ActionModelBrushExtrude::ActionModelBrushExtrude(const vec3 &_pos, const vec3 &_n, float _radius, float _depth) {
	pos = _pos;
	n = _n;
	radius = _radius;
	depth = _depth;
}

void* ActionModelBrushExtrude::execute(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	float r2 = radius * radius;

	for (int i=0;i<m->edit_mesh->vertex.num;i++) {
		float d2 = (pos - m->edit_mesh->vertex[i].pos).length_sqr();
		if (d2 < r2 * 2) {
			index.add(i);
			pos_old.add(m->edit_mesh->vertex[i].pos);
			m->edit_mesh->vertex[i].pos += n * depth * exp(- d2 / r2 * 2);
		}
	}
	m->set_normals_dirty_by_vertices(index);


	return NULL;
}

void ActionModelBrushExtrude::undo(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->set_normals_dirty_by_vertices(index);
	foreachi(int i, index, ii)
		m->edit_mesh->vertex[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
