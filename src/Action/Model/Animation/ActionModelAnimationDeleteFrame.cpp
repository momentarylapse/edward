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

ActionModelAnimationDeleteFrame::~ActionModelAnimationDeleteFrame()
{
}

void *ActionModelAnimationDeleteFrame::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->Move.num);
	assert(frame > 0);
	assert(frame < m->Move[index].Frame.num);

	old_frame = m->Move[index].Frame[frame];
	m->Move[index].Frame.erase(frame);
	return NULL;
}

void ActionModelAnimationDeleteFrame::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Move[index].Frame.insert(old_frame, frame);
}

