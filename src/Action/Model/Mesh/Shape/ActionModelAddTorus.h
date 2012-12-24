/*
 * ActionModelAddTorus.h
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTORUS_H_
#define ACTIONMODELADDTORUS_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/vector.h"

class ActionModelAddTorus: public ActionGroup
{
public:
	ActionModelAddTorus(const vector &_pos, const vector &_axis, float _radius1, float _radius2, int _num_x, int _num_y);
	virtual ~ActionModelAddTorus(){}
	string name(){	return "ModelAddTorus";	}

	void *compose(Data *d);
private:
	const vector &pos, &axis;
	float radius1, radius2;
	int num_x, num_y;
};

#endif /* ACTIONMODELADDTORUS_H_ */
