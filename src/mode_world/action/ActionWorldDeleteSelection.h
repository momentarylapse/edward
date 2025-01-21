/*
 * ActionWorldDeleteSelection.h
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDDELETESELECTION_H_
#define ACTIONWORLDDELETESELECTION_H_

#include <action/ActionGroup.h>
#include "../data/DataWorld.h"

class ActionWorldDeleteSelection : public ActionGroup {
public:
	explicit ActionWorldDeleteSelection(const base::set<void*>& selection);
	string name() override { return "WorldDeleteSelection";	}

	void *compose(Data *d) override;
	base::set<void*> selection;
};

#endif /* ACTIONWORLDDELETESELECTION_H_ */
