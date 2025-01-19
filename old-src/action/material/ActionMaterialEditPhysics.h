/*
 * ActionMaterialEditPhysics.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#ifndef ACTIONMATERIALEDITPHYSICS_H_
#define ACTIONMATERIALEDITPHYSICS_H_

#include "../Action.h"
#include "../../data/material/DataMaterial.h"

class ActionMaterialEditPhysics : public Action
{
public:
	ActionMaterialEditPhysics(const DataMaterial::PhysicsData &_phys);
	virtual ~ActionMaterialEditPhysics(){}
	string name(){	return "MaterialEditPhysics";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	DataMaterial::PhysicsData phys;
};

#endif /* ACTIONMATERIALEDITPHYSICS_H_ */
