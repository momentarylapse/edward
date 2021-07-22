/*
 * ActionAddVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDVERTEX_H_
#define ACTIONMODELADDVERTEX_H_

#include "../../../Action.h"
#include "../../../../lib/math/vector.h"
#include "../../../../lib/math/vec4.h"

class ActionModelAddVertex: public Action {
public:
	ActionModelAddVertex(const vector &_pos, const ivec4 &_bone_index = {0,0,0,0}, const vec4 &_bone_weight = {1,0,0,0}, int _normal_mode = -1);
	string name() override { return "ModelAddVertex"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	vector pos;
	ivec4 bone_index;
	vec4 bone_weight;
	int normal_mode;
};

#endif /* ACTIONMODELADDVERTEX_H_ */
