/*
 * Action.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTION_H_
#define ACTION_H_

#include "../Data/Data.h"

class Data;
class ActionManager;

class ActionException
{
public:
	ActionException(const string &_message){	message = _message;	}
	string message;
};

class Action
{
public:
	Action();
	virtual ~Action();

	virtual string name() = 0;

	virtual bool allowed(Data *d){	return true;	}
	virtual void abort(Data *d){	undo_logged(d);	}

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d){	execute(d);	}

	void *execute_logged(Data *d);
	void undo_logged(Data *d);
	void redo_logged(Data *d);

	void *execute_and_notify(Data *d);
	void undo_and_notify(Data *d);
	void redo_and_notify(Data *d);
};

#endif /* ACTION_H_ */
