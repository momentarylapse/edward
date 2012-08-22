/*
 * ActionModelAddCube.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDCUBE_H_
#define ACTIONMODELADDCUBE_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelAddCube: public ActionGroup
{
public:
	ActionModelAddCube(DataModel *m, const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3, int num_1, int num_2, int num_3);
	virtual ~ActionModelAddCube();
	string name(){	return "ModelAddCube";	}

	virtual void *execute_return(Data *d);
};

#endif /* ACTIONMODELADDCUBE_H_ */
