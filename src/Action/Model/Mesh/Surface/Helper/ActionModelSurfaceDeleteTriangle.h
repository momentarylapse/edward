/*
 * ActionModelSurfaceDeleteTriangle.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEDELETETRIANGLE_H_
#define ACTIONMODELSURFACEDELETETRIANGLE_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"
#include "../../../../../lib/x/x.h"

class ActionModelSurfaceDeleteTriangle: public Action
{
public:
	ActionModelSurfaceDeleteTriangle(int _surface, int _index);
	virtual ~ActionModelSurfaceDeleteTriangle();
	string name(){	return "Model__SurfaceDeleteTriangle";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface, index;
	int material, vertex[3];
	vector skin[3][MODEL_MAX_TEXTURES];
};

#endif /* ACTIONMODELSURFACEDELETETRIANGLE_H_ */
