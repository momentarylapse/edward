/*
 * ActionModelAddPlane.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDPLANE_H_
#define ACTIONMODELADDPLANE_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/vector.h"

class ActionModelAddPlane: public ActionGroup
{
public:
	ActionModelAddPlane(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y);
	virtual ~ActionModelAddPlane();
	string name(){	return "ModelAddPlane";	}

	void *compose(Data *d);
private:
	vector pos, dv1, dv2;
	int num_x, num_y;
};

#endif /* ACTIONMODELADDPLANE_H_ */
