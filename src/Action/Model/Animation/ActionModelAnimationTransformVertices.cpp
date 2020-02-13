/*
 * ActionModelAnimationTransformVertices.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/ModelMesh.h"
#include "../../../Mode/Model/Animation/ModeModelAnimation.h"
#include <assert.h>
#include "ActionModelAnimationTransformVertices.h"

ActionModelAnimationTransformVertices::ActionModelAnimationTransformVertices(DataModel *d, int _move, int _frame) :
	ActionMultiView()
{
	move = _move;
	frame = _frame;

	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->mesh->vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->move[move].frame[frame].vertex_dpos[i]);
		}
}

void* ActionModelAnimationTransformVertices::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->move[move].frame[frame].vertex_dpos[i] = mat * (old_data[ii] + m->mesh->vertex[i].pos) - m->mesh->vertex[i].pos;
	return NULL;
}

void ActionModelAnimationTransformVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->move[move].frame[frame].vertex_dpos[i] = old_data[ii];
}

