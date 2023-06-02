/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"

//#define ACTION_DEBUG



void ActionException::add_parent(const string &loc)
{
	location.insert(loc, 0);
}

string ActionException::where() const
{
	return implode(location, " > ");
}

Action::Action()
{
}

Action::~Action()
{
}

const string &Action::message()
{
	return Observable<VirtualBase>::MESSAGE_CHANGE;
}

void* Action::execute_logged(Data* d)
{
#ifdef ACTION_DEBUG
	msg_write("do " + name());
#endif
	void *r = execute(d);
	if (d->action_manager->enabled)
		d->notify(message());
#ifdef ACTION_DEBUG
	d->TestSanity("do " + name());
#endif
	return r;
}

void Action::undo_logged(Data* d)
{
#ifdef ACTION_DEBUG
	msg_write("undo " + name());
#endif
	undo(d);
	d->notify(message());
#ifdef ACTION_DEBUG
	d->TestSanity("undo " + name());
#endif
}

void Action::redo_logged(Data* d)
{
#ifdef ACTION_DEBUG
	msg_write("redo " + name());
#endif
	redo(d);
	d->notify(message());
#ifdef ACTION_DEBUG
	d->TestSanity("redo " + name());
#endif
}



