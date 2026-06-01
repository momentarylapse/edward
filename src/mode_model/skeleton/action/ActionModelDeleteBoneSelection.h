/*
 * ActionModelDeleteBoneSelection.h
 *
 *  Created on: 20.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEBONESELECTION_H_
#define ACTIONMODELDELETEBONESELECTION_H_

#include <lib/history/ActionGroup.h>
#include "../../data/DataModel.h"

class ActionModelDeleteBoneSelection: public history::ActionGroup {
public:
	explicit ActionModelDeleteBoneSelection(DataModel *m, const base::set<int>& sel);
	string name() const override { return "ModelDeleteBoneSelection"; }
};

#endif /* ACTIONMODELDELETEBONESELECTION_H_ */
