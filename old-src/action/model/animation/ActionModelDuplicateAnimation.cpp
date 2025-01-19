/*
 * ActionModelDuplicateAnimation.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ActionModelDuplicateAnimation.h"
#include "../../../data/model/DataModel.h"
#include <assert.h>

ActionModelDuplicateAnimation::ActionModelDuplicateAnimation(int _source, int _target)
{
	source = _source;
	target = _target;
}

void *ActionModelDuplicateAnimation::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(source >= 0);
	assert(source < m->move.num);
	assert(target >= 0);
	ModelMove &s = m->move[source];
	assert(s.frame.num > 0);

	// add animation "descriptor"
	if (m->move.num < target + 1)
		m->move.resize(target + 1);
	assert(m->move[target].frame.num == 0);

	m->move[target] = m->move[source];

	return NULL;
}

void ActionModelDuplicateAnimation::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->move[target].frame.clear();
}

