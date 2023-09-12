/*
 * ActionModelAddMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include "ActionModelAddMaterial.h"
#include "../../../data/model/DataModel.h"

ActionModelAddMaterial::ActionModelAddMaterial(const Path &_filename) {
	filename = _filename;
}

void *ActionModelAddMaterial::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	auto mat = new ModelMaterial(d->session, filename);
	mat->texture_levels.add(new ModelMaterial::TextureLevel());
	mat->texture_levels[0]->reload_image(d->session);

	m->material.add(mat);
	m->out_material_changed.notify();
	return &m->material.back();
}



void ActionModelAddMaterial::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	delete m->material.pop();
	m->out_material_changed.notify();
}


