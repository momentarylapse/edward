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
}

void* ActionModelEditEffect::execute(Data* d)
{
}

void ActionModelEditEffect::undo(Data* d)
{
}


