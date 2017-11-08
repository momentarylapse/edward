/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"

const string Data::MESSAGE_SELECTION = "Selection";

Data::Data(int _type) :
	Observable("Data")
{
	type = _type;
	action_manager = new ActionManager(this);
	file_time = 0;
	binary_file_format = false;
}

Data::~Data()
{
	delete(action_manager);
}



void Data::beginActionGroup(const string &name)
{
	action_manager->beginActionGroup(name);
}

void Data::endActionGroup()
{
	action_manager->endActionGroup();
}

void Data::redo()
{
	action_manager->redo();
}



void Data::undo()
{
	action_manager->undo();
}



void *Data::execute(Action *a)
{
	return action_manager->execute(a);
}



void Data::resetHistory()
{
	action_manager->reset();
}




