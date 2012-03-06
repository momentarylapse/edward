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

bool Data::Save()
{
	msg_todo("Data.Save");
	return true;
}



bool Data::Open()
{
	msg_todo("Data.Open");
	return true;
}



bool Data::SaveAs()
{
	msg_todo("Data.SaveAs");
	return true;
}



void Data::Redo()
{
	action_manager->redo();
}



void Data::Undo()
{
	action_manager->undo();
}



void *Data::Execute(Action *a)
{
	return action_manager->execute(a);
}



void Data::ResetHistory()
{
	action_manager->reset();
}




