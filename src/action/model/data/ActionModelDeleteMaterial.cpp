/*
 * ActionModelDeleteMaterial.cpp
 *
 *  Created on: Dec 16, 2020
 *      Author: michi
 */

#include "ActionModelDeleteMaterial.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/ModelPolygon.h"
#include <assert.h>

ActionModelDeleteMaterial::ActionModelDeleteMaterial(int _index) {
	index = _index;
	mat = nullptr;
}



void *ActionModelDeleteMaterial::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);

	mat = m->material[index];
	m->material.erase(index);

	// correct polygons
	for (auto &p: m->mesh->polygon) {
		assert(p.material != index);
		if (p.material > index)
			p.material --;
	}

	m->out_material_changed.notify();
	return nullptr;
}



void ActionModelDeleteMaterial::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);

	m->material.insert(mat, index);

	// correct polygons
	for (auto &p: m->mesh->polygon) {
		if (p.material >= index)
			p.material ++;
	}

	m->out_material_changed.notify();
}

