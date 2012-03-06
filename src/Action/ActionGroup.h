/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONGROUP_H_
#define ACTIONGROUP_H_

#include "Action.h"
#include "../Data/Data.h"

class Data;

class ActionGroup: public Action
{
public:
	ActionGroup();
	virtual ~ActionGroup();

	bool is_atom();
	virtual bool needs_preparation() = 0;

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);
	virtual void prepare(Data *d) = 0;

protected:
	Array<Action*> action;
};

#endif /* ACTIONGROUP_H_ */
