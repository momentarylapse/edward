/*
 * ActionModelSetNormalModeAll.h
 *
 *  Created on: 08.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETNORMALMODEALL_H_
#define ACTIONMODELSETNORMALMODEALL_H_

#include "../Action.h"
#include "../../lib/types/types.h"
#include "../../Data/Model/DataModel.h"

class ActionModelSetNormalModeAll : public Action
{
public:
	ActionModelSetNormalModeAll(int _mode);
	virtual ~ActionModelSetNormalModeAll();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int mode;
	Array<int> old_mode;
	int old_mode_all;
};

#endif /* ACTIONMODELSETNORMALMODEALL_H_ */
