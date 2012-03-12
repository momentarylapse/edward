/*
 * ActionModelDeleteSelection.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETESELECTION_H_
#define ACTIONMODELDELETESELECTION_H_

#include "../Action.h"
#include "../ActionGroup.h"
#include "../../lib/types/types.h"

class ActionModelDeleteSelection: public ActionGroup
{
public:
	ActionModelDeleteSelection();
	virtual ~ActionModelDeleteSelection();
};

#endif /* ACTIONMODELDELETESELECTION_H_ */
