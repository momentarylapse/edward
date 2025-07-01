/*
 * ActionModelAddMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include "ActionModelAddMaterial.h"
#include "../../../data/DataModel.h"

ActionModelAddMaterial::ActionModelAddMaterial(xfer<ModelMaterial> m) {
	material = m;
}

void *ActionModelAddMaterial::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->materials.add(material.get());
	m->out_material_changed.notify();
	return &m->materials.back();
}



void ActionModelAddMaterial::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->materials.pop();
	m->out_material_changed.notify();
}


