/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONGROUP_H_
#define ACTIONGROUP_H_

#include "Action.h"
#include "../lib/file/file.h"

class Data;
class ActionManager;

class ActionGroup: public Action
{
	friend class ActionManager;
public:
	ActionGroup();
	virtual ~ActionGroup();

	virtual string name(){	return "-group-";	}

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

protected:
	void *AddSubAction(Action *a, Data *d);
	virtual void *execute_return(Data *d);

private:
	Array<Action*> action;
};

#endif /* ACTIONGROUP_H_ */
