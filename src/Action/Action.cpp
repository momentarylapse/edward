/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"

#define ACTION_DEBUG

Action::Action()
{
}

Action::~Action()
{
}

void* Action::execute_logged(Data* d)
{
#ifdef ACTION_DEBUG
	msg_write("do " + name());
#endif
	void *r = execute(d);
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
#ifdef ACTION_DEBUG
	d->TestSanity("redo " + name());
#endif
}



