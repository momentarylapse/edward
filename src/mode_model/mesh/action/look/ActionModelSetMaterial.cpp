/*
 * ActionModelSetMaterial.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetMaterial.h"

#include <Session.h>

#include "../../../data/DataModel.h"
#include "../../../data/ModelMesh.h"
#include <data/mesh/Polygon.h>

ActionModelSetMaterial::ActionModelSetMaterial(DataModel *m, const Data::Selection& sel, int _material) {
	material = _material;

	// save old data
	foreachi(Polygon &t, m->mesh->polygons, ti)
		if (sel[MultiViewType::MODEL_POLYGON].contains(ti)) {
			triangle.add(ti);
			old_material.add(t.material);
		}
}


void *ActionModelSetMaterial::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);


	for (int i: triangle) {
		Polygon &t = m->mesh->polygons[i];
		t.material = material;
	}
	return NULL;
}



void ActionModelSetMaterial::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi (int i, triangle, k) {
		Polygon &t = m->mesh->polygons[i];
		t.material = old_material[k];
	}
}


