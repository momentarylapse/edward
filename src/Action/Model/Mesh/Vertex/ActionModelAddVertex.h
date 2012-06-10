/*
 * ActionAddVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDVERTEX_H_
#define ACTIONMODELADDVERTEX_H_

#include "../../../Action.h"
#include "../../../../lib/types/types.h"

class ActionModelAddVertex: public Action
{
public:
	ActionModelAddVertex(const vector &_v, int _normal_mode = -1);
	virtual ~ActionModelAddVertex();
	string name(){	return "ModelAddVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector v;
	int normal_mode;
};

#endif /* ACTIONMODELADDVERTEX_H_ */
