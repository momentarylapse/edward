/*
 * ActionModelMoveVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelMoveVertex.h"
#include "../../../../../data/model/DataModel.h"
#include "../../../../../data/model/ModelMesh.h"
#include "../../../../../data/model/ModelPolygon.h"
#include <assert.h>

ActionModelMoveVertex::ActionModelMoveVertex(int _vertex, const vec3 &_pos) {
	vertex = _vertex;
	pos = _pos;
}

void *ActionModelMoveVertex::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	std::swap(m->edit_mesh->vertex[vertex].pos, pos);
	return NULL;
}

void ActionModelMoveVertex::undo(Data *d) {
	execute(d);
}
