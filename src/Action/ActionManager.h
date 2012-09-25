/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "Action.h"
#include "../Data/Data.h"
#include "../Stuff/Observable.h"

class Data;
class Action;
class ActionGroup;

class ActionManager : public Observable
{
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();
	void Reset();
	void Enable(bool _enabled);

	void *Execute(Action *a);
	void Undo();
	void Redo();

	void BeginActionGroup();
	void EndActionGroup();

	bool Undoable();
	bool Redoable();
	bool IsSave();
	void MarkCurrentAsSave();

	string error_message;

private:
	void add(Action *a);
	Data *data;
	Array<Action*> action;
	int cur_pos;
	int save_pos;

	int cur_level;
	bool enabled;

	// group
	int cur_group_level;
	ActionGroup *cur_group;
};

#endif /* ACTIONMANAGER_H_ */
