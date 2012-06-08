/*
 * ActionModelSetNormalModeSelection.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETNORMALMODESELECTION_H_
#define ACTIONMODELSETNORMALMODESELECTION_H_

#include "../Action.h"
#include "../../lib/types/types.h"
#include "../../Data/Model/DataModel.h"

class ActionModelSetNormalModeSelection: public Action
{
public:
	ActionModelSetNormalModeSelection(DataModel *m, int _mode);
	virtual ~ActionModelSetNormalModeSelection();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int mode;
	Array<int> index;
	Array<int> old_mode;
	int old_mode_all;
};

#endif /* ACTIONMODELSETNORMALMODESELECTION_H_ */
