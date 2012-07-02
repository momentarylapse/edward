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
class ActionGroup;

class ActionManager
{
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();
	void Reset();

	void *Execute(Action *a);
	void add(Action *a);
	void Undo();
	void Redo();

	void BeginActionGroup();
	void EndActionGroup();

	bool Undoable();
	bool Redoable();
	bool IsSave();
	void MarkCurrentAsSave();

	Data *data;

	Array<Action*> action;
	int cur_pos;
	int save_pos;

	// group
	int cur_group_level;
	ActionGroup *cur_group;
};

#endif /* ACTIONMANAGER_H_ */
