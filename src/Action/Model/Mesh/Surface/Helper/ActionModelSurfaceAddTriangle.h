/*
 * ActionModelSurfaceAddTriangle.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEADDTRIANGLE_H_
#define ACTIONMODELSURFACEADDTRIANGLE_H_

#include "../../../../Action.h"
class vector;

class ActionModelSurfaceAddTriangle: public Action
{
public:
	ActionModelSurfaceAddTriangle(int _surface, Array<int> &_v, int _material, Array<vector> &_sv, int _index = -1);
	virtual ~ActionModelSurfaceAddTriangle();
	string name(){	return "ModelSurfaceAddTriangle";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
	int index;
	Array<int> v;
	int material;
	Array<vector> sv;
};

#endif /* ACTIONMODELSURFACEADDTRIANGLE_H_ */
