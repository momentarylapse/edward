/*
 * ActionModelDeleteTriangle.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETETRIANGLE_H_
#define ACTIONMODELDELETETRIANGLE_H_

#include "../../../Action.h"

class ActionModelDeleteTriangle: public Action
{
public:
	ActionModelDeleteTriangle(int _surface, int _triangle);
	virtual ~ActionModelDeleteTriangle();
	string name(){	return "ModelDeleteTriangle";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
};

#endif /* ACTIONMODELDELETETRIANGLE_H_ */
