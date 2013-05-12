/*
 * ActionModelAutomap.cpp
 *
 *  Created on: 12.05.2013
 *      Author: michi
 */

#include "ActionModelAutomap.h"

ActionModelAutomap::ActionModelAutomap(int _material, int _texture_level)
{
	material = _material;
	texture_level = _texture_level;
}

ActionModelAutomap::~ActionModelAutomap()
{
}

void *ActionModelAutomap::execute(Data *d)
{
}

void ActionModelAutomap::undo(Data *d)
{
}

