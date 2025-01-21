/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "ActionGroup.h"
#include "../data/Data.h"
#include "../lib/os/msg.h"
#include <assert.h>

const string ActionManager::MESSAGE_FAILED = "failed";
const string ActionManager::MESSAGE_SAVED = "saved";

ActionManager::ActionManager(Data *_data) {
	data = _data;
	cur_pos = 0;
	save_pos = 0;
	cur_group_level = 0;
	cur_group = NULL;
	_preview = NULL;
	enabled = true;
	cur_level = 0;
}

ActionManager::~ActionManager() {
	reset();
}

void ActionManager::reset() {
	for (Action *a: action)
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

void ActionManager::enable(bool _enabled) {
	enabled = _enabled;
}

void ActionManager::add(Action *a) {
	if (!enabled)
		return;
	// truncate history
	for (int i=cur_pos;i<action.num;i++)
		delete(action[i]);
	action.resize(cur_pos);

	action.add(a);
	cur_pos ++;
}



void *ActionManager::execute(Action *a) {
	clear_preview();
	error_message = "";

	if (cur_group)
		return cur_group->addSubAction(a, data);

	try {
		void *p = a->execute_logged(data);
		if (!a->was_trivial())
			add(a);
		if (enabled)
			data->out_changed();
		if (!cur_group)
			data->on_post_action_update();
		return p;
	} catch(ActionException &e) {
		e.add_parent(a->name());
		error_message = e.message;
		error_location = e.where();
		msg_error(error_message);
		msg_write("at " + error_location);
		a->abort(data);
		out_failed.notify();
		return nullptr;
	}
}

void ActionManager::undo() {
	clear_preview();
	if (undoable()) {
		action[-- cur_pos]->undo_logged(data);
		data->on_post_action_update();
	}
}



void ActionManager::redo() {
	clear_preview();
	if (redoable()) {
		action[cur_pos ++]->redo_logged(data);
		data->on_post_action_update();
	}
}

bool ActionManager::undoable() {
	return (cur_pos > 0);
}



bool ActionManager::redoable()
{
	return (cur_pos < action.num);
}



void ActionManager::begin_group(const string &name)
{
	clear_preview();
	if (!cur_group){
		cur_group = new ActionGroupManual(name);
	}
	cur_group_level ++;
}

void ActionManager::end_group()
{
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0){
		ActionGroup *g = cur_group;
		cur_group = NULL;
		execute(g);
		data->on_post_action_update();
	}
}

void ActionManager::mark_current_as_save() {
	save_pos = cur_pos;
	out_saved.notify();
}



bool ActionManager::is_save() {
	return (cur_pos == save_pos);
}


bool ActionManager::preview(Action *a)
{
	clear_preview();
	try{
		a->execute_logged(data);
		data->on_post_action_update();
		_preview = a;
	}catch(ActionException &e){
		e.add_parent(a->name());
		error_message = e.message;
		error_location = e.where();
		a->abort(data);
		delete(a);
		out_failed.notify();
		return false;
	}
	return true;
}


void ActionManager::clear_preview() {
	if (_preview) {
		_preview->undo_logged(data);
		data->on_post_action_update();
		delete(_preview);
		_preview = NULL;
	}
}
