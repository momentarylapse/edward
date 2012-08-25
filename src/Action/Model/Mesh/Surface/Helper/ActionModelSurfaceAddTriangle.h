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
#include "../../../../../lib/x/x.h"

class ActionModelSurfaceAddTriangle: public Action
{
public:
	ActionModelSurfaceAddTriangle(int _surface, int _a, int _b, int _c, int _material, const vector *_sva, const vector *_svb, const vector *_svc);
	virtual ~ActionModelSurfaceAddTriangle();
	string name(){	return "ModelSurfaceAddTriangle";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
	int a, b, c;
	int material;
	vector sv[3][MODEL_MAX_TEXTURES];
};

#endif /* ACTIONMODELSURFACEADDTRIANGLE_H_ */
