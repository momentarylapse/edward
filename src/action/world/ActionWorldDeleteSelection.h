/*
 * ActionWorldDeleteSelection.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETESELECTION_H_
#define ACTIONWORLDDELETESELECTION_H_

#include "../ActionGroup.h"
#include "../../data/world/DataWorld.h"

class ActionWorldDeleteSelection : public ActionGroup {
public:
	ActionWorldDeleteSelection();
	string name(){	return "WorldDeleteSelection";	}

	void *compose(Data *d);
};

#endif /* ACTIONWORLDDELETESELECTION_H_ */
