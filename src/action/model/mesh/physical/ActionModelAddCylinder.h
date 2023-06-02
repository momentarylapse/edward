/*
 * ActionModelAddCylinder.h
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELADDCYLINDER_H_
#define SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELADDCYLINDER_H_


#include "../../../Action.h"
#include "../../../../data/model/ModelMesh.h"

class ActionModelAddCylinder: public Action {
public:
	ActionModelAddCylinder(const ModelCylinder &c);
	string name(){ return "ModelAddCylinder"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	ModelCylinder cylinder;
};

#endif /* SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELADDCYLINDER_H_ */
