/*
 * ActionModelBrushExtrude.h
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBRUSHEXTRUDE_H_
#define ACTIONMODELBRUSHEXTRUDE_H_

#include "../../../Action.h"
#include "../../../../lib/math/math.h"
class DataModel;

class ActionModelBrushExtrude: public Action
{
public:
	ActionModelBrushExtrude(const vector &pos, const vector &n, float radius, float depth);
	virtual ~ActionModelBrushExtrude();
	string name(){	return "ModelBrushExtrude";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos, n;
	float radius, depth;
	Array<int> index;
	Array<vector> pos_old;
};

#endif /* ACTIONMODELBRUSHEXTRUDE_H_ */
