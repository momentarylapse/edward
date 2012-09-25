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
	d->NotifyBegin();
	undo_logged(d);
	d->Notify("Change");
	d->NotifyEnd();
}



void *Action::execute_and_notify(Data *d)
{
	void *r = NULL;
	try{
		d->NotifyBegin();
		r = execute_logged(d);
		d->Notify("Change");
		d->NotifyEnd();
	}catch(ActionException &e){
		// needed to end notify block even when action failed
		d->NotifyEnd();
		throw;
	}
	return r;
}



void Action::redo_and_notify(Data *d)
{
	d->NotifyBegin();
	redo_logged(d);
	d->Notify("Change");
	d->NotifyEnd();
}

void* Action::execute_logged(Data* d)
{
	msg_write("do " + name());
	return execute(d);
}

void Action::undo_logged(Data* d)
{
	msg_write("undo " + name());
	undo(d);
}

void Action::redo_logged(Data* d)
{
	msg_write("redo " + name());
	redo(d);
}



