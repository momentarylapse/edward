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

	ModelMaterial mat = ModelMaterial(filename);
	m->material.add(mat);
	return &m->material.back();
}



void ActionModelAddMaterial::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->material.pop();
}


