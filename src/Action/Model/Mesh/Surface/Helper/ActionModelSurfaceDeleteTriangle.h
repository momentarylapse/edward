/*
 * ActionModelSurfaceDeleteTriangle.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEDELETETRIANGLE_H_
#define ACTIONMODELSURFACEDELETETRIANGLE_H_

#include "../../../../Action.h"
class vector;

class ActionModelSurfaceDeleteTriangle: public Action
{
public:
	ActionModelSurfaceDeleteTriangle(int _surface, int _index);
	virtual ~ActionModelSurfaceDeleteTriangle();
	string name(){	return "ModelSurfaceDeleteTriangle";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface, index;
	int material;
	Array<int> vertex;
	Array<vector> skin;
};

#endif /* ACTIONMODELSURFACEDELETETRIANGLE_H_ */
