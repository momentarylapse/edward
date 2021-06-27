/*
 * ActionModelBrushSmooth.h
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBRUSHSMOOTH_H_
#define ACTIONMODELBRUSHSMOOTH_H_

#include "../../../Action.h"
#include "../../../../lib/math/vector.h"
class DataModel;

class ActionModelBrushSmooth: public Action {
public:
	ActionModelBrushSmooth(const vector &pos, const vector &n, float radius);
	string name(){	return "ModelBrushSmooth";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos, n;
	float radius;
	Array<int> index;
	Array<vector> pos_old;
};

#endif /* ACTIONMODELBRUSHSMOOTH_H_ */
