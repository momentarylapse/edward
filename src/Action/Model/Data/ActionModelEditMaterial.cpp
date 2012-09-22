/*
 * ActionModelEditMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include "ActionModelEditMaterial.h"
#include <assert.h>

ActionModelEditMaterial::ActionModelEditMaterial(int _index, const ModelMaterial &_mat)
{
	index = _index;
	mat = _mat;
}

ActionModelEditMaterial::~ActionModelEditMaterial()
{
}

void *ActionModelEditMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert((index >= 0) && (index < m->Material.num));

	ModelMaterial old_mat = m->Material[index];
	m->Material[index] = mat;
	mat = old_mat;

	return NULL;
}



void ActionModelEditMaterial::undo(Data *d)
{
	execute(d);
}


