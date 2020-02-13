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
class ModelSelectionState;

class ActionModelDeleteSelection: public ActionGroup {
public:
	ActionModelDeleteSelection(const ModelSelectionState &s, bool _greedy);
	string name() override { return "ModelDeleteSelection"; }

	void *compose(Data *d) override;
private:
	bool greedy;
	const ModelSelectionState &sel;
};

#endif /* ACTIONMODELDELETESELECTION_H_ */
