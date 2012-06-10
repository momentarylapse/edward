/*
 * ActionModelAddMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDMATERIAL_H_
#define ACTIONMODELADDMATERIAL_H_

#include "../../Action.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelAddMaterial : public Action
{
public:
	ActionModelAddMaterial();
	virtual ~ActionModelAddMaterial();
	string name(){	return "ModelAddMaterial";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELADDMATERIAL_H_ */
