/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"

Action::Action()
{
}

Action::~Action()
{
}

void Action::undo_and_notify(Data *d)
{
	msg_write("undo " + name());
	d->NotifyBegin();
	undo(d);
	d->Notify("Change");
	d->NotifyEnd();
}



void *Action::execute_and_notify(Data *d)
{
	msg_write("do " + name());
	d->NotifyBegin();
	void *r = execute(d);
	d->Notify("Change");
	d->NotifyEnd();
	return r;
}



void Action::redo_and_notify(Data *d)
{
	msg_write("redo " + name());
	d->NotifyBegin();
	redo(d);
	d->Notify("Change");
	d->NotifyEnd();
}


