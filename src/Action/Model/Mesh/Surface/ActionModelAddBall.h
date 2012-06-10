/*
 * ActionModelAddBall.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDBALL_H_
#define ACTIONMODELADDBALL_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelAddBall: public ActionGroup
{
public:
	ActionModelAddBall(DataModel *m, const vector &_pos, float _radius, int _num_x, int _num_y, bool _as_sphere);
	virtual ~ActionModelAddBall();
	string name(){	return "ModelAddBall";	}
};

#endif /* ACTIONMODELADDBALL_H_ */
