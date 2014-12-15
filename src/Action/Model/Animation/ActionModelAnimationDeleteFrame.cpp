/*
 * ActionModelAnimationDeleteFrame.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationDeleteFrame.h"
#include <assert.h>

ActionModelAnimationDeleteFrame::ActionModelAnimationDeleteFrame(int _index, int _frame)
{
	index = _index;
	frame = _frame;
}

void *ActionModelAnimationDeleteFrame::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	assert(frame >= 0);
	assert(frame < m->move[index].frame.num);

	old_frame = m->move[index].frame[frame];
	m->move[index].frame.erase(frame);

	return NULL;
}

void ActionModelAnimationDeleteFrame::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->move[index].frame.insert(old_frame, frame);
}

