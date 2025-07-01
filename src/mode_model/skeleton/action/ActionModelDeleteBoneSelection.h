/*
 * ActionModelDeleteBoneSelection.h
 *
 *  Created on: 20.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEBONESELECTION_H_
#define ACTIONMODELDELETEBONESELECTION_H_

#include <action/ActionGroup.h>
#include "../../data/DataModel.h"

class ActionModelDeleteBoneSelection: public ActionGroup {
public:
	explicit ActionModelDeleteBoneSelection(DataModel *m, const base::set<int>& sel);
	string name() override { return "ModelDeleteBoneSelection"; }
};

#endif /* ACTIONMODELDELETEBONESELECTION_H_ */
