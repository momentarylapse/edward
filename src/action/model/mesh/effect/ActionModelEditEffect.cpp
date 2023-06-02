/*
 * ActionModelEditEffect.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelEditEffect.h"

ActionModelEditEffect::ActionModelEditEffect(int _index, const ModelEffect& _effect)
{
	index = _index;
	effect = _effect;
}

void* ActionModelEditEffect::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelEffect temp = m->fx[index];
	effect.vertex = temp.vertex;
	m->fx[index] = effect;
	effect = temp;

	return &m->fx[index];
}

void ActionModelEditEffect::undo(Data* d)
{
	execute(d);
}


