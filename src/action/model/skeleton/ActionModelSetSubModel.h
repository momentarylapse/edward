/*
 * ActionModelSetSubModel.h
 *
 *  Created on: Dec 16, 2014
 *      Author: ankele
 */

#ifndef SRC_ACTION_MODEL_SKELETON_ACTIONMODELSETSUBMODEL_H_
#define SRC_ACTION_MODEL_SKELETON_ACTIONMODELSETSUBMODEL_H_

#include "../../Action.h"
#include "../../../lib/math/math.h"

class Model;

class ActionModelSetSubModel: public Action
{
public:
	ActionModelSetSubModel(int index, const Path &filename);
	virtual ~ActionModelSetSubModel();
	string name(){	return "ModelSetSubModel";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	Path filename;
	Model *model;
};

#endif /* SRC_ACTION_MODEL_SKELETON_ACTIONMODELSETSUBMODEL_H_ */
