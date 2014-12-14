/*
 * ActionModelAnimationTransformVertices.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "../../../Data/Model/DataModel.h"
#include "../../../Mode/Model/Animation/ModeModelAnimation.h"
#include <assert.h>
#include "ActionModelAnimationTransformVertices.h"

ActionModelAnimationTransformVertices::ActionModelAnimationTransformVertices(DataModel *d, int _move, int _frame) :
	ActionMultiView()
{
	move = _move;
	frame = _frame;

	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->Move[move].Frame[frame].VertexDPos[i]);
		}
}

void* ActionModelAnimationTransformVertices::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].VertexDPos[i] = mat * (old_data[ii] + m->Vertex[i].pos) - m->Vertex[i].pos;
	return NULL;
}

void ActionModelAnimationTransformVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].VertexDPos[i] = old_data[ii];
}

