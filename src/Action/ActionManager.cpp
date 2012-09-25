/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "ActionGroup.h"
#include "../Data/Data.h"
#include <assert.h>

ActionManager::ActionManager(Data *_data) :
	Observable("ActionManager")
{
	data = _data;
	cur_pos = 0;
	save_pos = 0;
	cur_group_level = 0;
	cur_group = NULL;
}

ActionManager::~ActionManager()
{
	Reset();
}

void ActionManager::Reset()
{
	foreach(Action *a, action)
		delete(a);
	action.clear();
	cur_pos = 0;
	save_pos = 0;
	cur_group_level = 0;
	if (cur_group)
		delete(cur_group);
	cur_group = NULL;
}



void ActionManager::add(Action *a)
{
	// truncate history
	for (int i=cur_pos;i<action.num;i++)
		delete(action[i]);
	action.resize(cur_pos);

	action.add(a);
	cur_pos ++;
}



void *ActionManager::Execute(Action *a)
{
	error_message = "";

	if (cur_group)
		return cur_group->AddSubAction(a, data);

	try{
		data->NotifyBegin();
		void *p = a->execute_logged(data);
		add(a);
		data->Notify("Change");
		data->NotifyEnd();
		return p;
	}catch(ActionException &e){
		data->NotifyEnd();
		error_message = e.message;
		msg_error("ActionManager: " + e.message);
		a->abort(data);
		Notify("Failed");
		return NULL;
	}
}



void ActionManager::Undo()
{
	if (Undoable()){
		data->NotifyBegin();
		action[-- cur_pos]->undo_logged(data);
		data->Notify("Change");
		data->NotifyEnd();
	}
}



void ActionManager::Redo()
{
	if (Redoable()){
		data->NotifyBegin();
		action[cur_pos ++]->redo_logged(data);
		data->Notify("Change");
		data->NotifyEnd();
	}
}

bool ActionManager::Undoable()
{
	return (cur_pos > 0);
}



bool ActionManager::Redoable()
{
	return (cur_pos < action.num);
}



void ActionManager::BeginActionGroup()
{
	if (!cur_group){
		cur_group = new ActionGroup;
	}
	cur_group_level ++;
}

void ActionManager::EndActionGroup()
{
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0){
		ActionGroup *g = cur_group;
		cur_group = NULL;
		Execute(g);
		data->Notify("Change");
	}
}

void ActionManager::MarkCurrentAsSave()
{
	save_pos = cur_pos;
	data->Notify("Change");
}



bool ActionManager::IsSave()
{
	return (cur_pos == save_pos);
}




