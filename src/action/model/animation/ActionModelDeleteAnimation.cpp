/*
 * ActionModelDeleteAnimation.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelDeleteAnimation.h"
#include <assert.h>

ActionModelDeleteAnimation::ActionModelDeleteAnimation(int _index)
{
	index = _index;
}

void *ActionModelDeleteAnimation::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	assert(m->move[index].frame.num > 0);
	animation = m->move[index];
	m->move[index].frame.clear();

	return NULL;
}

void ActionModelDeleteAnimation::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	assert(m->move[index].frame.num == 0);
	m->move[index] = animation;
}

