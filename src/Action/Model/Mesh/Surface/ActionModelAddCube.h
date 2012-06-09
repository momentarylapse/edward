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
	ActionModelAddCube(DataModel *m, const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3);
	virtual ~ActionModelAddCube();
};

#endif /* ACTIONMODELADDCUBE_H_ */
