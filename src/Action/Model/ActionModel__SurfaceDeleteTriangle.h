/*
 * ActionModel__SurfaceDeleteTriangle.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODEL__SURFACEDELETETRIANGLE_H_
#define ACTIONMODEL__SURFACEDELETETRIANGLE_H_

#include "../Action.h"
#include "../../lib/types/types.h"
#include "../../lib/x/x.h"

class ActionModel__SurfaceDeleteTriangle: public Action
{
public:
	ActionModel__SurfaceDeleteTriangle(int _surface, int _index);
	virtual ~ActionModel__SurfaceDeleteTriangle();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface, index;
	int material, vertex[3];
	vector skin[3][MODEL_MAX_TEXTURES];
};

#endif /* ACTIONMODEL__SURFACEDELETETRIANGLE_H_ */
