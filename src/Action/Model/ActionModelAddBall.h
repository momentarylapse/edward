/*
 * ActionModelAddBall.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDBALL_H_
#define ACTIONMODELADDBALL_H_

#include "../Action.h"
#include "../ActionGroup.h"
#include "../../lib/types/types.h"

class ActionModelAddBall: public ActionGroup
{
public:
	ActionModelAddBall(const vector &_pos, float _radius, int _num_x, int _num_y, bool _as_sphere, int nv);
	virtual ~ActionModelAddBall();

	bool needs_preparation();
	void prepare(Data *d);
};

#endif /* ACTIONMODELADDBALL_H_ */
