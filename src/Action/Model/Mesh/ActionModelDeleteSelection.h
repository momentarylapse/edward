/*
 * ActionModelDeleteSelection.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETESELECTION_H_
#define ACTIONMODELDELETESELECTION_H_

#include "../../ActionGroup.h"
class DataModel;

class ActionModelDeleteSelection: public ActionGroup
{
public:
	ActionModelDeleteSelection(DataModel *m, bool greedy);
	virtual ~ActionModelDeleteSelection();
	string name(){	return "ModelDeleteSelection";	}
};

#endif /* ACTIONMODELDELETESELECTION_H_ */
