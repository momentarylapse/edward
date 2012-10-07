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

	virtual void Reset() = 0;
	virtual bool Load(const string &_filename, bool deep = true) = 0;
	virtual bool Save(const string &_filename) = 0;

	void ResetHistory();
	void *Execute(Action *a);
	void BeginActionGroup();
	void EndActionGroup();
	void Undo();
	void Redo();

	virtual bool TestSanity(const string &loc){	return true;	}

	string filename;
	int file_time;
	bool binary_file_format;
	int type;

	ActionManager *action_manager;
};

#endif /* DATA_H_ */
