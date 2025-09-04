/*
 * ActionModelBrushExtrude.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ActionModelBrushExtrude.h"
#include <mode_model/data/DataModel.h>
#include <mode_model/data/ModelMesh.h>
#include <lib/base/iter.h>
#include <cmath>

ActionModelBrushExtrude::ActionModelBrushExtrude(const vec3& _pos, const vec3& _n, float _radius, float _depth) {
	pos = _pos;
	n = _n;
	radius = _radius;
	depth = _depth;
}

void* ActionModelBrushExtrude::execute(Data* d) {
	auto m = dynamic_cast<DataModel*>(d);

	float r2 = radius * radius;

	for (auto&& [i, v]: enumerate(m->mesh->vertices)) {
		float d2 = (pos - m->mesh->vertices[i].pos).length_sqr();
		if (d2 < r2 * 2) {
			index.add(i);
			pos_old.add(m->mesh->vertices[i].pos);
			m->mesh->vertices[i].pos += n * depth * exp(- d2 / r2 * 2);
		}
	}
	//m->set_normals_dirty_by_vertices(index);

	return nullptr;
}

void ActionModelBrushExtrude::undo(Data* d) {
	auto m = dynamic_cast<DataModel*>(d);

	//m->set_normals_dirty_by_vertices(index);
	for (const auto& [ii, i]: enumerate(index))
		m->mesh->vertices[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
