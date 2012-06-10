/*
 * ActionModelSplitTriangle.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSPLITTRIANGLE_H_
#define ACTIONMODELSPLITTRIANGLE_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelSplitTriangle: public ActionGroup
{
public:
	ActionModelSplitTriangle(DataModel *m, int _surface, int _triangle, const vector &_pos);
	virtual ~ActionModelSplitTriangle();
	string name(){	return "ModelSplitTriangle";	}
};

#endif /* ACTIONMODELSPLITTRIANGLE_H_ */
