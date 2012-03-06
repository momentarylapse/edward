/*
 * ActionAtom.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONATOM_H_
#define ACTIONATOM_H_

#include "Action.h"
#include "../Data/Data.h"

class Data;

class ActionAtom: public Action
{
public:
	ActionAtom();
	virtual ~ActionAtom();

	bool is_atom();
	virtual bool needs_preparation() = 0;

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d) = 0;
	virtual void prepare(Data *d) = 0;
};

#endif /* ACTIONATOM_H_ */
