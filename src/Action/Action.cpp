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



