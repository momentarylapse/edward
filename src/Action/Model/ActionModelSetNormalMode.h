/*
 * ActionModelSetNormalMode.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETNORMALMODE_H_
#define ACTIONMODELSETNORMALMODE_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionModelSetNormalMode: public Action
{
public:
	ActionModelSetNormalMode(int _mode);
	virtual ~ActionModelSetNormalMode();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	Array<int> index;
	Array<int> old_mode;
};

#endif /* ACTIONMODELSETNORMALMODE_H_ */
