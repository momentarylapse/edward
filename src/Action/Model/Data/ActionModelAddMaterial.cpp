/*
 * ActionModelAddMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include "ActionModelAddMaterial.h"
#include "../../../Data/Model/DataModel.h"

ActionModelAddMaterial::ActionModelAddMaterial(const string &_filename)
{
	filename = _filename;
}

ActionModelAddMaterial::~ActionModelAddMaterial()
{
}

void *ActionModelAddMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	auto mat = new ModelMaterial(filename);
	mat->texture_levels.add(new ModelMaterial::TextureLevel());
	mat->texture_levels[0]->reload_image();

	m->material.add(mat);
	m->notify(m->MESSAGE_MATERIAL_CHANGE);
	return &m->material.back();
}



void ActionModelAddMaterial::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	delete m->material.pop();
	m->notify(m->MESSAGE_MATERIAL_CHANGE);
}


