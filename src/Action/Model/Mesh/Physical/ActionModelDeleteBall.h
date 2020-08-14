/*
 * ActionModelDeleteBall.h
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELDELETEBALL_H_
#define SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELDELETEBALL_H_

#include "../../../Action.h"
#include "../../../../Data/Model/ModelMesh.h"

class ActionModelDeleteBall: public Action {
public:
	ActionModelDeleteBall(int index);
	string name(){ return "ModelDeleteBall"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	ModelBall ball;
	int index;
};

#endif /* SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELDELETEBALL_H_ */
