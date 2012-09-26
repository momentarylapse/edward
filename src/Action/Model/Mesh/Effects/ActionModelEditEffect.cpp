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

	ModelEffect temp = m->Fx[index];
	effect.Vertex = temp.Vertex;
	m->Fx[index] = effect;
	effect = temp;

	return &m->Fx[index];
}

void ActionModelEditEffect::undo(Data* d)
{
	execute(d);
}


