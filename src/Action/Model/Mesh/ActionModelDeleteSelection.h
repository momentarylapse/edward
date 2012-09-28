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
	ActionModelDeleteSelection(bool _greedy);
	virtual ~ActionModelDeleteSelection(){}
	string name(){	return "ModelDeleteSelection";	}

	void *compose(Data *d);
private:
	bool greedy;
};

#endif /* ACTIONMODELDELETESELECTION_H_ */
