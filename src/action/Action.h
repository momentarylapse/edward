/*
 * Action.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTION_H_
#define ACTION_H_

#include "../data/Data.h"

class Data;
class ActionManager;

class ActionException
{
public:
	ActionException(const string &_message){	message = _message;	}
	string message;
	Array<string> location;
	void add_parent(const string &loc);
	string where() const;
};

class Action
{
public:
	Action();
	virtual ~Action();

	virtual string name() = 0;
	//virtual const string &message();

	virtual bool allowed(Data *d){	return true;	}
	virtual void abort(Data *d){	undo_logged(d);	}
	virtual bool was_trivial(){	return false;	}

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d){	execute(d);	}

	void *execute_logged(Data *d);
	void undo_logged(Data *d);
	void redo_logged(Data *d);
};

#endif /* ACTION_H_ */
