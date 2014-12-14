/*
 * ActionModelAddAnimation.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAddAnimation.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAddAnimation::ActionModelAddAnimation(int _index, int _type)
{
	index = _index;
	type = _type;
}

void *ActionModelAddAnimation::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	if (m->Move.num >= index + 1)
		assert(m->Move[index].Frame.num == 0);

	// add animation "descriptor"
	if (m->Move.num < index + 1)
		m->Move.resize(index + 1);
	m->Move[index].Frame.clear();
	m->Move[index].Name = "";
	m->Move[index].Type = type;
	m->Move[index].FramesPerSecConst = 2;
	m->Move[index].FramesPerSecFactor = 0;

	// add animation data
	ModelFrame f;
	if (type ==MoveTypeSkeletal){
		f.SkelAng.resize(m->Bone.num);
		f.SkelDPos.resize(m->Bone.num);
	}else if (type ==MoveTypeVertex){
		f.VertexDPos.resize(m->Vertex.num);
	}
	m->Move[index].Frame.add(f);
	return NULL;
}

void ActionModelAddAnimation::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Move[index].Frame.clear();
}

