/*
 * ActionModelAddCube.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDCUBE_H_
#define ACTIONMODELADDCUBE_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"

class ActionModelAddCube: public ActionGroup
{
public:
	ActionModelAddCube(const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3, int num_1, int num_2, int num_3);
	virtual ~ActionModelAddCube();
	string name(){	return "ModelAddCube";	}

	void *compose(Data *d);
private:
	vector pos, dv[3];
	int num[3];
};

#endif /* ACTIONMODELADDCUBE_H_ */
