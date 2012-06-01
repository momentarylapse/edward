/*
 * ActionModelAddCylinder.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDCYLINDER_H_
#define ACTIONMODELADDCYLINDER_H_

#include "../Action.h"
#include "../ActionGroup.h"
#include "../../lib/types/types.h"
#include "../../Data/Model/DataModel.h"

class ActionModelAddCylinder: public ActionGroup
{
public:
	ActionModelAddCylinder(DataModel *m, Array<vector> &pos, float radius, int rings, int edges, bool closed);
	virtual ~ActionModelAddCylinder();
};

#endif /* ACTIONMODELADDCYLINDER_H_ */
