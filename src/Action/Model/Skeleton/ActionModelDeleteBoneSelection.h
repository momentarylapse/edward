/*
 * ActionModelDeleteBoneSelection.h
 *
 *  Created on: 20.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEBONESELECTION_H_
#define ACTIONMODELDELETEBONESELECTION_H_

#include "../../Action.h"
#include "../../ActionGroup.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelDeleteBoneSelection: public ActionGroup
{
public:
	ActionModelDeleteBoneSelection(DataModel *m);
	virtual ~ActionModelDeleteBoneSelection();
	string name(){	return "ModelDeleteBoneSelection";	}
};

#endif /* ACTIONMODELDELETEBONESELECTION_H_ */
