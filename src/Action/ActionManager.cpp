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

const string ActionManager::MESSAGE_FAILED = "Failed";
const string ActionManager::MESSAGE_SAVED = "Saved";

ActionManager::ActionManager(Data *_data) :
	Observable("ActionManager")
{
	data = _data;
	cur_pos = 0;
	save_pos = 0;
	cur_group_level = 0;
	cur_group = NULL;
	_preview = NULL;
}

ActionManager::~ActionManager()
{
	reset();
}

void ActionManager::reset()
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
	_preview = NULL;
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



void *ActionManager::execute(Action *a)
{
	clearPreview();
	error_message = "";

	if (cur_group)
		return cur_group->addSubAction(a, data);

	try{
		data->notifyBegin();
		void *p = a->execute_logged(data);
		if (!a->was_trivial())
			add(a);
		if (!cur_group){
			data->onPostActionUpdate();
		}
		data->notifyEnd();
		return p;
	}catch(ActionException &e){
		data->notifyEnd();
		e.add_parent(a->name());
		error_message = e.message;
		error_location = e.where();
		msg_error(error_message);
		msg_write("at " + error_location);
		a->abort(data);
		notify(MESSAGE_FAILED);
		return NULL;
	}
}



void ActionManager::undo()
{
	clearPreview();
	if (undoable()){
		data->notifyBegin();
		action[-- cur_pos]->undo_logged(data);
		data->onPostActionUpdate();
		data->notifyEnd();
	}
}



void ActionManager::redo()
{
	clearPreview();
	if (redoable()){
		data->notifyBegin();
		action[cur_pos ++]->redo_logged(data);
		data->onPostActionUpdate();
		data->notifyEnd();
	}
}

bool ActionManager::undoable()
{
	return (cur_pos > 0);
}



bool ActionManager::redoable()
{
	return (cur_pos < action.num);
}



void ActionManager::beginActionGroup(const string &name)
{
	clearPreview();
	if (!cur_group){
		cur_group = new ActionGroupManual(name);
	}
	cur_group_level ++;
}

void ActionManager::endActionGroup()
{
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0){
		ActionGroup *g = cur_group;
		cur_group = NULL;
		execute(g);
		data->onPostActionUpdate();
	}
}

void ActionManager::markCurrentAsSave()
{
	save_pos = cur_pos;
	notify(MESSAGE_SAVED);
}



bool ActionManager::isSave()
{
	return (cur_pos == save_pos);
}


bool ActionManager::preview(Action *a)
{
	clearPreview();
	try{
		data->notifyBegin();
		a->execute_logged(data);
		data->notifyEnd();
		_preview = a;
	}catch(ActionException &e){
		data->notifyEnd();
		e.add_parent(a->name());
		error_message = e.message;
		error_location = e.where();
		a->abort(data);
		delete(a);
		notify(MESSAGE_FAILED);
		return false;
	}
	return true;
}


void ActionManager::clearPreview()
{
	if (_preview){
		data->notifyBegin();
		_preview->undo_logged(data);
		data->notifyEnd();
		delete(_preview);
		_preview = NULL;
	}
}
