/*
 * ActionModelBrushExtrudeVertices.h
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBRUSHEXTRUDEVERTICES_H_
#define ACTIONMODELBRUSHEXTRUDEVERTICES_H_

#include "../../../Action.h"
#include "../../../../lib/math/math.h"
class DataModel;

class ActionModelBrushExtrudeVertices: public Action
{
public:
	ActionModelBrushExtrudeVertices(const vector &pos, const vector &n, float radius, float depth);
	virtual ~ActionModelBrushExtrudeVertices();
	string name(){	return "ModelBrushExtrudeVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos, n;
	float radius, depth;
	Array<int> index;
	Array<vector> pos_old;
};

#endif /* ACTIONMODELBRUSHEXTRUDEVERTICES_H_ */
