/*
 * ActionModelInvertSelection.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELINVERTSELECTION_H_
#define ACTIONMODELINVERTSELECTION_H_

#include "../../../ActionGroup.h"
class DataModel;

class ActionModelInvertSelection : public ActionGroup
{
public:
	ActionModelInvertSelection();
	virtual ~ActionModelInvertSelection(){}
	string name(){	return "ModelInvertSelection";	}

	void *compose(Data *d);
};

#endif /* ACTIONMODELINVERTSELECTION_H_ */
