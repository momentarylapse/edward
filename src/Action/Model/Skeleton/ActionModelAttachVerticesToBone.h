/*
 * ActionModelAttachVerticesToBone.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELATTACHVERTICESTOBONE_H_
#define ACTIONMODELATTACHVERTICESTOBONE_H_

#include "../../Action.h"
#include "../../../lib/math/vector.h"

class ActionModelAttachVerticesToBone: public Action {
public:
	ActionModelAttachVerticesToBone(const Array<int> &index, const Array<ivec4> &bone, const Array<vec4> &weight);
	string name() override { return "ModelAttachVerticesToBone"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Array<int> index;
	Array<ivec4> bone;
	Array<vec4> weight;
};

#endif /* ACTIONMODELATTACHVERTICESTOBONE_H_ */
