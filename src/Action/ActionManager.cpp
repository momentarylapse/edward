/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "../Data/Data.h"

ActionManager::ActionManager(Data *_data)
{
	data = _data;
	cur_action = 0;
}

ActionManager::~ActionManager()
{
	foreach(action, a)
		delete(a);
}



void ActionManager::add(Action *a)
{
	// truncate history
	for (int i=cur_action;i<action.num;i++)
		delete(action[i]);
	action.resize(cur_action);

	action.add(a);
	cur_action ++;
}



void ActionManager::execute(Action *a)
{
	add(a);
	a->execute(data);
}



void ActionManager::undo()
{
	if (cur_action > 0)
		action[-- cur_action]->undo(data);
}



void ActionManager::redo()
{
	if (cur_action < action.num)
		action[cur_action ++]->redo(data);
}


