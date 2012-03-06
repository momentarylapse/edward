/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "../lib/file/file.h"
#include "Action.h"
#include "../Data/Data.h"

class Data;
class Action;

class ActionManager
{
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();
	void reset();

	void *execute(Action *a);
	void add(Action *a);
	void undo();
	void redo();

	Data *data;

	Array<Action*> action;
	int cur_action;
};

#endif /* ACTIONMANAGER_H_ */
