/*
 * ActionModelAnimationMoveVertices.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationMoveVertices.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAnimationMoveVertices::ActionModelAnimationMoveVertices(Data* d, const vector& _pos0):
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	move = m->CurrentMove;
	frame = m->CurrentFrame;

	// list of selected vertices and save old pos
	foreachi(m->Vertex, v, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(m->Move[move].Frame[frame].VertexDPos[i]);
		}
}

ActionModelAnimationMoveVertices::~ActionModelAnimationMoveVertices()
{
}

void* ActionModelAnimationMoveVertices::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Move[move].Frame[frame].VertexDPos[i] = old_data[ii] + param;
	m->UpdateAnimation();
	return NULL;
}

void ActionModelAnimationMoveVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Move[move].Frame[frame].VertexDPos[i] = old_data[ii];
	m->UpdateAnimation();
}
