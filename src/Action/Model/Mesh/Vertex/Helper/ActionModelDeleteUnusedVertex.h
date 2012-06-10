/*
 * ActionModelDeleteUnusedVertex.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEUNUSEDVERTEX_H_
#define ACTIONMODELDELETEUNUSEDVERTEX_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"

class ActionModelDeleteUnusedVertex: public Action
{
public:
	ActionModelDeleteUnusedVertex(int _vertex);
	virtual ~ActionModelDeleteUnusedVertex();
	string name(){	return "ModelDeleteUnusedVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vector pos;
	int bone, normal_mode;
	Array<vector> move;
};

#endif /* ACTIONMODELDELETEUNUSEDVERTEX_H_ */
