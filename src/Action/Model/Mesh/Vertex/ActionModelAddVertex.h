/*
 * ActionAddVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDVERTEX_H_
#define ACTIONMODELADDVERTEX_H_

#include "../../../Action.h"
#include "../../../../lib/math/math.h"

class ActionModelAddVertex: public Action {
public:
	ActionModelAddVertex(const vector &_pos, int _bone_index = 0, int _normal_mode = -1);
	string name(){ return "ModelAddVertex"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos;
	int bone_index;
	int normal_mode;
};

#endif /* ACTIONMODELADDVERTEX_H_ */
