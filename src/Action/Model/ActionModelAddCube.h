/*
 * ActionModelAddCube.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDCUBE_H_
#define ACTIONMODELADDCUBE_H_

#include "../Action.h"
#include "../ActionGroup.h"
#include "../../lib/types/types.h"

class ActionModelAddCube: public ActionGroup
{
public:
	ActionModelAddCube(const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3, int nv);
	virtual ~ActionModelAddCube();

	bool needs_preparation();
	void prepare(Data *d);
};

#endif /* ACTIONMODELADDCUBE_H_ */
