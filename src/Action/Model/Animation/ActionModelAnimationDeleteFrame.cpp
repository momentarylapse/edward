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
	assert(frame >= 0);
	assert(frame < m->Move[index].Frame.num);

	old_frame = m->Move[index].Frame[frame];
	m->Move[index].Frame.erase(frame);

	// make current state valid
	if (index == m->CurrentMove){
		if (m->Move[index].Frame.num == 0)
			m->SetCurrentMove(-1);
		else if (m->CurrentFrame >= m->Move[index].Frame.num)
			m->SetCurrentFrame(m->CurrentFrame - 1);
	}
	return NULL;
}

void ActionModelAnimationDeleteFrame::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Move[index].Frame.insert(old_frame, frame);
	m->UpdateAnimation();
}

