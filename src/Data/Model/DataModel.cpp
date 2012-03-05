/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "../../Action/Action.h"
#include "../../Action/ActionManager.h"

DataModel::DataModel()
{
	action_manager = new ActionManager(this);
}

DataModel::~DataModel()
{
}



void DataModel::Reset()
{
}



void DataModel::Load(const string & _filename, bool deep)
{
}



void DataModel::Save(const string & _filename)
{
}


