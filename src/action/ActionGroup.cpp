/*
 * ActionGroup.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionGroup.h"

ActionGroup::ActionGroup()
{
}

ActionGroup::~ActionGroup()
{
	for (Action *a: action)
		delete(a);
	action.clear();
}

void *ActionGroup::addSubAction(Action *a, Data *d)
{
	void *r = NULL;
	try{
		r = a->execute_logged(d);
		if (!a->was_trivial())
			action.add(a);
	}catch(ActionException &e){
		e.add_parent(a->name());
		a->abort(d);
		throw;
	}
	return r;
}


void *ActionGroup::execute(Data *d)
{
	void *r = compose(d);

	// no need to execute sub actions ... done during compose()
	/*for (action, a)
		a->execute_logged(d);*/
	return r;
}



void ActionGroup::undo(Data *d)
{
	foreachb(Action *a, action)
		a->undo_logged(d);
}



void ActionGroup::redo(Data *d)
{
	for (Action *a: action)
		a->redo_logged(d);
}

void ActionGroup::abort(Data *d)
{
	foreachb(Action *a, action)
		a->undo_logged(d);
}

bool ActionGroup::was_trivial()
{
	return action.num == 0;
}


