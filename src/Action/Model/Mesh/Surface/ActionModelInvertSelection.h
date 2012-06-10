/*
 * ActionModelInvertSelection.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELINVERTSELECTION_H_
#define ACTIONMODELINVERTSELECTION_H_

#include "../../../ActionGroup.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelInvertSelection : public ActionGroup
{
public:
	ActionModelInvertSelection(DataModel *m);
	virtual ~ActionModelInvertSelection();
	string name(){	return "ModelInvertSelection";	}
};

#endif /* ACTIONMODELINVERTSELECTION_H_ */
