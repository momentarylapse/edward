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

ActionModelDeleteAnimation::~ActionModelDeleteAnimation()
{
}

void *ActionModelDeleteAnimation::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->Move.num);
	assert(m->Move[index].Frame.num > 0);
	animation = m->Move[index];
	m->Move[index].Frame.clear();

	if (index == m->CurrentMove)
		m->SetCurrentMove(-1);
	return NULL;
}

void ActionModelDeleteAnimation::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->Move.num);
	assert(m->Move[index].Frame.num == 0);
	m->Move[index] = animation;
}

