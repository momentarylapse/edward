/*
 * Action.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTION_H_
#define ACTION_H_

#include "../Data/Data.h"

class Data;

class Action
{
public:
	Action();
	virtual ~Action();

	virtual bool is_atom() = 0;
	virtual bool needs_preparation() = 0;

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d) = 0;
	virtual void prepare(Data *d) = 0;
};

#endif /* ACTION_H_ */
