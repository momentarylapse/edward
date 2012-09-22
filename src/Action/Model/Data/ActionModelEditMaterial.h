/*
 * ActionModelEditMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITMATERIAL_H_
#define ACTIONMODELEDITMATERIAL_H_

#include "../../Action.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelEditMaterial : public Action
{
public:
	ActionModelEditMaterial(int _index, const ModelMaterial &_mat);
	virtual ~ActionModelEditMaterial();
	string name(){	return "ModelEditMaterial";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	ModelMaterial mat;
};

#endif /* ACTIONMODELEDITMATERIAL_H_ */
