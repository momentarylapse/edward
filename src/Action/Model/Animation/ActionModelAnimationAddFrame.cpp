/*
 * ActionModelAnimationAddFrame.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationAddFrame.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAnimationAddFrame::ActionModelAnimationAddFrame(int _index, int _frame)
{
	index = _index;
	frame = _frame;
}

void *ActionModelAnimationAddFrame::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	assert(frame >= 0);
	assert(frame <= m->move[index].frame.num);

	ModelFrame new_frame;
	if (frame > 0)
		new_frame = m->move[index].frame[frame - 1];
	else
		new_frame = m->move[index].frame[0];
	m->move[index].frame.insert(new_frame, frame);
	return NULL;
}

void ActionModelAnimationAddFrame::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->move[index].frame.erase(frame);
}

