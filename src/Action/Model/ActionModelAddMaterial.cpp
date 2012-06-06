/*
 * ActionModelAddMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include "ActionModelAddMaterial.h"
#include "../../Data/Model/DataModel.h"

ActionModelAddMaterial::ActionModelAddMaterial()
{
}

ActionModelAddMaterial::~ActionModelAddMaterial()
{
}

void *ActionModelAddMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelMaterial mat;
	m->Material.add(mat);
	return &m->Material.back();
}



void ActionModelAddMaterial::redo(Data *d)
{
	execute(d);
}



void ActionModelAddMaterial::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->Material.pop();
	if (m->CurrentMaterial >= m->Material.num)
		m->CurrentMaterial = m->Material.num - 1;
}


