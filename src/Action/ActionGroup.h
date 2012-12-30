/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONGROUP_H_
#define ACTIONGROUP_H_

#include "Action.h"

class Data;
class ActionManager;

class ActionGroup: public Action
{
	friend class ActionManager;
public:
	ActionGroup();
	virtual ~ActionGroup();

	virtual string name() = 0;

	virtual void *compose(Data *d){	return NULL;	}

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	virtual void abort(Data *d);
	virtual bool was_trivial();

protected:
	void *AddSubAction(Action *a, Data *d);

private:
	Array<Action*> action;
};

class ActionGroupManual : public ActionGroup
{
	friend class ActionManager;
public:
	ActionGroupManual(const string &name){	_name_ = name;	}
	virtual ~ActionGroupManual(){}

	virtual string name(){	return _name_;	}
private:
	string _name_;
};

#endif /* ACTIONGROUP_H_ */
