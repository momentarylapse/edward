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

class ActionManager : public Observable<VirtualBase> {
	friend class Action;
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();

	static const string MESSAGE_FAILED;
	static const string MESSAGE_SAVED;

	void reset();
	void enable(bool _enabled);

	void *execute(Action *a);
	void undo();
	void redo();

	void begin_group(const string &name);
	void end_group();

	bool preview(Action *a);
	void clear_preview();

	bool undoable();
	bool redoable();
	bool is_save();
	void mark_current_as_save();

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
