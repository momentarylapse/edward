/*
 * ActionModelAddBall.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDBALL_H_
#define ACTIONMODELADDBALL_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/vector.h"

class ActionModelAddBall: public ActionGroup
{
public:
	ActionModelAddBall(const vector &_pos, float _radius, int _num_x, int _num_y);
	virtual ~ActionModelAddBall(){}
	string name(){	return "ModelAddBall";	}

	void *compose(Data *d);
private:
	vector pos;
	float radius;
	int num_x, num_y;
};

#endif /* ACTIONMODELADDBALL_H_ */
