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
	foreach(Action *a, action)
		delete(a);
	action.clear();
}

void *ActionGroup::AddSubAction(Action *a, Data *d)
{
	action.add(a);
	return a->execute_logged(d);
}


void *ActionGroup::execute(Data *d)
{
	void *r = compose(d);

	// no need to execute sub actions ... done during compose()
	/*foreach(action, a)
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
	foreach(Action *a, action)
		a->redo_logged(d);
}

void ActionGroup::abort(Data *d)
{
	foreachb(Action *a, action)
		a->abort(d);
}


