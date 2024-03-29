/*
 * ActionModelDeleteVertex.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEVERTEX_H_
#define ACTIONMODELDELETEVERTEX_H_

#include "../../../../lib/math/vec3.h"
#include "../../../Action.h"

class ActionModelDeleteVertex: public Action {
public:
	ActionModelDeleteVertex(int _vertex);
	string name(){	return "ModelDeleteVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vec3 pos;
	int normal_mode;
};

#endif /* ACTIONMODELDELETEVERTEX_H_ */
