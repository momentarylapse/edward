/*
 * ActionModelAnimationMoveVertices.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationMoveVertices.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Mode/Model/Animation/ModeModelAnimation.h"
#include <assert.h>

ActionModelAnimationMoveVertices::ActionModelAnimationMoveVertices(DataModel *d, const vector &_param, const vector &_pos0) :
	ActionMultiView()
{
	move = mode_model_animation->CurrentMove;
	frame = mode_model_animation->CurrentFrame;

	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->Move[move].Frame[frame].VertexDPos[i]);
		}
}

ActionModelAnimationMoveVertices::~ActionModelAnimationMoveVertices()
{
}

void* ActionModelAnimationMoveVertices::execute(Data* d)
{
	/*DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].VertexDPos[i] = old_data[ii] + param;*/
	return NULL;
}

void ActionModelAnimationMoveVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].VertexDPos[i] = old_data[ii];
}
