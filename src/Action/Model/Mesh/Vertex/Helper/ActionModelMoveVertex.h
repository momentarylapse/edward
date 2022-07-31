/*
 * ActionModelMoveVertex.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVEVERTEX_H_
#define ACTIONMODELMOVEVERTEX_H_

#include "../../../../Action.h"
#include "../../../../../lib/math/vec3.h"
class DataModel;

class ActionModelMoveVertex: public Action {
public:
	ActionModelMoveVertex(int _vertex, const vec3 &_pos);
	string name(){	return "ModelMoveVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vec3 pos;
};

#endif /* ACTIONMODELMOVEVERTEX_H_ */
