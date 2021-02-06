/*
 * ActionModelSetNormalModeSelection.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETNORMALMODESELECTION_H_
#define ACTIONMODELSETNORMALMODESELECTION_H_

#include "../../../Action.h"
class DataModel;

class ActionModelSetNormalModeSelection: public Action {
public:
	ActionModelSetNormalModeSelection(DataModel *m, int _mode);
	string name() override { return "ModelSetNormalModeSelection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int mode;
	Array<int> index;
	Array<int> old_mode;

	Array<int> p_index;
	Array<int> old_group;
	int smooth_group;
};

#endif /* ACTIONMODELSETNORMALMODESELECTION_H_ */
