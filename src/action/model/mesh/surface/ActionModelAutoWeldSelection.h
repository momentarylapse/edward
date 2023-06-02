/*
 * ActionModelAutoWeldSelection.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELAUTOWELDSELECTION_H_
#define ACTIONMODELAUTOWELDSELECTION_H_

#include "../../../ActionGroup.h"
class DataModel;

class ActionModelAutoWeldSelection : public ActionGroup
{
public:
	ActionModelAutoWeldSelection(float _epsilon);
	virtual ~ActionModelAutoWeldSelection(){}
	string name(){	return "ModelAutoWeldSelection";	}

	void *compose(Data *d);
private:
	float epsilon;
};

#endif /* ACTIONMODELAUTOWELDSELECTION_H_ */
