/*
 * ActionModelDeleteVertex.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEVERTEX_H_
#define ACTIONMODELDELETEVERTEX_H_

#include "../../../Action.h"
#include "../../../../lib/math/math.h"

class ActionModelDeleteVertex: public Action
{
public:
	ActionModelDeleteVertex(int _vertex);
	virtual ~ActionModelDeleteVertex();
	string name(){	return "ModelDeleteVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vector pos;
	int normal_mode;
};

#endif /* ACTIONMODELDELETEVERTEX_H_ */
