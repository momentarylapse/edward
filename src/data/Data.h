/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATA_H_
#define DATA_H_

#include "../lib/base/base.h"
#include "../lib/base/set.h"
#include "../lib/base/map.h"
#include "../lib/os/path.h"
#include "../lib/pattern/Observable.h"
#include "../action/Action.h"
#include "../action/ActionManager.h"

enum class MultiViewType;
class ActionManager;
class Action;
class Session;

class Data : public obs::Node<VirtualBase> {
public:
	Data(Session *s, int _type);
	~Data() override;

	//obs::source out_selection{this, "selection"};

	virtual void reset() = 0;

	void reset_history();
	void *execute(Action *a);
	void begin_action_group(const string &name);
	void end_action_group();
	void undo();
	void redo();

	virtual void on_post_action_update() {}

	Path filename;
	int file_time;
	bool binary_file_format;
	int type;

	Session *session;
	ActionManager *action_manager;

	//using Selection = base::set<const void*>;
	using Selection = base::map<MultiViewType, base::set<int>>;
	virtual Selection get_selection() const { return {}; }
};

#endif /* DATA_H_ */
