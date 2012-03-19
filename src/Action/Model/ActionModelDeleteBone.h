/*
 * ActionModelDeleteBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEBONE_H_
#define ACTIONMODELDELETEBONE_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionModelDeleteBone: public Action
{
public:
	ActionModelDeleteBone(int _index);
	virtual ~ActionModelDeleteBone();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int index;
	Array<int> child;
	vector pos;
	int parent;
	string filename;
	void *model;
};

#endif /* ACTIONMODELDELETEBONE_H_ */
