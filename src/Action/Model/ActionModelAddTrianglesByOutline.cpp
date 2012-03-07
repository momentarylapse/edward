/*
 * ActionModelAddTrianglesByOutline.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ActionModelAddTrianglesByOutline.h"
#include "ActionModelAddVertex.h"
#include "ActionModelAddTriangle.h"

ActionModelAddTrianglesByOutline::ActionModelAddTrianglesByOutline(Array<int> vertex, DataModel *data)
{
	msg_write("add trias...");
}

ActionModelAddTrianglesByOutline::~ActionModelAddTrianglesByOutline()
{
}

bool ActionModelAddTrianglesByOutline::needs_preparation()
{	return false;	}

void ActionModelAddTrianglesByOutline::prepare(Data *d)
{
}
