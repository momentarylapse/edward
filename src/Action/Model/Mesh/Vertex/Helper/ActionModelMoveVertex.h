/*
 * ActionModelMoveVertex.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVEVERTEX_H_
#define ACTIONMODELMOVEVERTEX_H_

#include "../../../../Action.h"
#include "../../../../../lib/math/vector.h"
class DataModel;

class ActionModelMoveVertex: public Action {
public:
	ActionModelMoveVertex(int _vertex, const vector &_pos);
	string name(){	return "ModelMoveVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vector pos;
};

#endif /* ACTIONMODELMOVEVERTEX_H_ */
