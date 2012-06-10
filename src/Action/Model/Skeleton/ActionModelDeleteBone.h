/*
 * ActionModelDeleteBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEBONE_H_
#define ACTIONMODELDELETEBONE_H_

#include "../../Action.h"
#include "../../../lib/types/types.h"

class ActionModelDeleteBone: public Action
{
public:
	ActionModelDeleteBone(int _index);
	virtual ~ActionModelDeleteBone();
	string name(){	return "ModelDeleteBone";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	Array<int> child;
	vector pos;
	int parent;
	string filename;
	void *model;
	Array<vector> move_dpos;
	Array<vector> move_ang;
	Array<int> vertex;
};

#endif /* ACTIONMODELDELETEBONE_H_ */
