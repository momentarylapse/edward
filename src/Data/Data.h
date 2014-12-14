/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATA_H_
#define DATA_H_

#include "../lib/file/file.h"
#include "../Action/Action.h"
#include "../Action/ActionManager.h"
#include "../Stuff/Observable.h"

class ActionManager;
class Action;
class Observable;

class Data : public Observable
{
public:
	Data(int _type);
	virtual ~Data();

	virtual void reset() = 0;
	virtual bool load(const string &_filename, bool deep = true) = 0;
	virtual bool save(const string &_filename) = 0;

	void resetHistory();
	void *execute(Action *a);
	void beginActionGroup(const string &name);
	void endActionGroup();
	void undo();
	void redo();

	virtual void onPostActionUpdate(){}
	virtual bool testSanity(const string &loc){	return true;	}

	string filename;
	int file_time;
	bool binary_file_format;
	int type;

	ActionManager *action_manager;
};

#endif /* DATA_H_ */
