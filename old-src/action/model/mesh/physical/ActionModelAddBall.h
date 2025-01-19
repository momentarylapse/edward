/*
 * ActionModelAddBall.h
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELADDBALL_H_
#define SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELADDBALL_H_

#include "../../../Action.h"
#include "../../../../data/model/ModelMesh.h"

class ActionModelAddBall: public Action {
public:
	ActionModelAddBall(const ModelBall &b);
	string name(){ return "ModelAddBall"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	ModelBall ball;
};

#endif /* SRC_ACTION_MODEL_MESH_PHYSICAL_ACTIONMODELADDBALL_H_ */
