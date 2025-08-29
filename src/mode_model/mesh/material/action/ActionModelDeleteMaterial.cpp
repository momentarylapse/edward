/*
 * ActionModelDeleteMaterial.cpp
 *
 *  Created on: Dec 16, 2020
 *      Author: michi
 */

#include "ActionModelDeleteMaterial.h"
#include "../../../data/DataModel.h"
#include "../../../data/ModelMesh.h"
#include <lib/mesh/Polygon.h>
#include <assert.h>

ActionModelDeleteMaterial::ActionModelDeleteMaterial(int _index) {
	index = _index;
	mat = nullptr;
}



void *ActionModelDeleteMaterial::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);

	mat = m->materials[index];
	m->materials.erase(index);

	// correct polygons
	for (auto &p: m->mesh->polygons) {
		assert(p.material != index);
		if (p.material > index)
			p.material --;
	}

	m->out_material_changed.notify();
	return nullptr;
}



void ActionModelDeleteMaterial::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);

	m->materials.insert(mat, index);

	// correct polygons
	for (auto &p: m->mesh->polygons) {
		if (p.material >= index)
			p.material ++;
	}

	m->out_material_changed.notify();
}

