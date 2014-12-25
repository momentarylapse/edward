/*
 * ActionModelAnimationSetFrameDuration.cpp
 *
 *  Created on: 25.12.2014
 *      Author: michi
 */

#include <algorithm>
#include "ActionModelAnimationSetFrameDuration.h"
#include <assert.h>

ActionModelAnimationSetFrameDuration::ActionModelAnimationSetFrameDuration(int _index, int _frame, float _duration)
{
	index = _index;
	frame = _frame;
	duration = _duration;
}

void* ActionModelAnimationSetFrameDuration::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	assert(frame >= 0);
	assert(frame < m->move[index].frame.num);

	std::swap(duration, m->move[index].frame[frame].duration);

	return NULL;
}

void ActionModelAnimationSetFrameDuration::undo(Data* d)
{
	execute(d);
}
