/*
 * ActionModel__SurfaceAddTriangle.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODEL__SURFACEADDTRIANGLE_H_
#define ACTIONMODEL__SURFACEADDTRIANGLE_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"
#include "../../../../../lib/x/x.h"

class ActionModel__SurfaceAddTriangle: public Action
{
public:
	ActionModel__SurfaceAddTriangle(int _surface, int _a, int _b, int _c, int _material, const vector *_sva, const vector *_svb, const vector *_svc);
	virtual ~ActionModel__SurfaceAddTriangle();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
	int a, b, c;
	int material;
	vector sv[3][MODEL_MAX_TEXTURES];
};

#endif /* ACTIONMODEL__SURFACEADDTRIANGLE_H_ */
