/*
 * ActionModelDeleteCylinder.h
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELDELETECYLINDER_H_
#define SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELDELETECYLINDER_H_

#include "../../../Action.h"
#include "../../../../Data/Model/ModelMesh.h"

class ActionModelDeleteCylinder: public Action {
public:
	ActionModelDeleteCylinder(int index);
	string name(){ return "ModelDeleteCylinder"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	ModelCylinder cylinder;
	int index;
};

#endif /* SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELDELETECYLINDER_H_ */
