/*
 * ActionModelAnimationAddFrame.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationAddFrame.h"
#include "../../../data/model/DataModel.h"
#include <y/world/components/Animator.h>
#include <assert.h>

ActionModelAnimationAddFrame::ActionModelAnimationAddFrame(int _index, int _frame, const ModelFrame &_f)
{
	index = _index;
	frame = _frame;
	f = _f;
}

void *ActionModelAnimationAddFrame::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	assert(frame >= 0);
	assert(frame <= m->move[index].frame.num);

	m->move[index].frame.insert(f, frame);
	return NULL;
}

void ActionModelAnimationAddFrame::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->move[index].frame.erase(frame);
}

