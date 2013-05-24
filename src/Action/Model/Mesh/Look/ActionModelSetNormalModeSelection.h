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

class ActionModelSetNormalModeSelection: public Action
{
public:
	ActionModelSetNormalModeSelection(DataModel *m, int _mode);
	virtual ~ActionModelSetNormalModeSelection();
	string name(){	return "ModelSetNormalModeSelection";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int mode;
	Array<int> index;
	Array<int> old_mode;
};

#endif /* ACTIONMODELSETNORMALMODESELECTION_H_ */
