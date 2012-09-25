/*
 * ActionMaterialEditPhysics.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionMaterialEditPhysics.h"

ActionMaterialEditPhysics::ActionMaterialEditPhysics(const DataMaterial::PhysicsData &_phys)
{
	phys = _phys;
}


void *ActionMaterialEditPhysics::execute(Data *d)
{
	DataMaterial *m = dynamic_cast<DataMaterial*>(d);

	DataMaterial::PhysicsData old_phys = m->Physics;

	m->Physics = phys;

	phys = old_phys;

	return NULL;
}



void ActionMaterialEditPhysics::undo(Data *d)
{
	execute(d);
}

