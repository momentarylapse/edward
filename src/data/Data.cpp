/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"

Data::Data(Session *_session, int _type) {
	session = _session;
	type = _type;
	action_manager = new ActionManager(this);
	file_time = 0;
	binary_file_format = false;
}

Data::~Data() {
	delete action_manager;
}



void Data::begin_action_group(const string &name) {
	action_manager->begin_group(name);
}

void Data::end_action_group() {
	action_manager->end_group();
}

void Data::redo() {
	action_manager->redo();
}



void Data::undo() {
	action_manager->undo();
}



void *Data::execute(Action *a) {
	return action_manager->execute(a);
}


void Data::reset_history() {
	action_manager->reset();
}


