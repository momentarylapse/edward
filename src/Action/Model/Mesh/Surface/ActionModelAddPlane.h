/*
 * ActionModelAddPlane.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDPLANE_H_
#define ACTIONMODELADDPLANE_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelAddPlane: public ActionGroup
{
public:
	ActionModelAddPlane(DataModel *m, const vector &_pos, const vector &_dv1, const vector &_dv2, int _num_x, int _num_y);
	virtual ~ActionModelAddPlane();
	string name(){	return "ModelAddPlane";	}
};

#endif /* ACTIONMODELADDPLANE_H_ */
