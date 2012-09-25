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
	ActionModelAutoWeldSelection(DataModel *m, float d);
	virtual ~ActionModelAutoWeldSelection();
	string name(){	return "ModelAutoWeldSelection";	}
};

#endif /* ACTIONMODELAUTOWELDSELECTION_H_ */
