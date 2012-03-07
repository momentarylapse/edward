/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"

Data::Data()
{
}

Data::~Data()
{
}



void Data::Redo()
{
	action_manager->Redo();
}



void Data::Undo()
{
	action_manager->Undo();
}



void *Data::Execute(Action *a)
{
	return action_manager->Execute(a);
}



void Data::ResetHistory()
{
	action_manager->Reset();
}




