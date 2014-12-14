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
	void reset();
	void enable(bool _enabled);

	void *execute(Action *a);
	void undo();
	void redo();

	void beginActionGroup(const string &name);
	void endActionGroup();

	bool preview(Action *a);
	void clearPreview();

	bool undoable();
	bool redoable();
	bool isSave();
	void markCurrentAsSave();

	string error_message;
	string error_location;

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

	// preview
	Action *_preview;
};

#endif /* ACTIONMANAGER_H_ */
