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

class ActionManager;
class Action;

class Data
{
public:
	Data();
	virtual ~Data();

	bool Open();
	bool Save();
	bool SaveAs();

	virtual void Reset() = 0;
	virtual void Load(const string &_filename, bool deep = true) = 0;
	virtual void Save(const string &_filename) = 0;

	void Execute(Action *a);
	void Undo();
	void Redo();

	string filename;

	ActionManager *action_manager;
};

#endif /* DATA_H_ */
