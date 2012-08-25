/*
 * ActionModelInvertSelection.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelInvertSelection.h"
#include "ActionModelSurfaceInvert.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelInvertSelection::ActionModelInvertSelection(DataModel *m)
{
	foreachi(m->Surface, s, i)
		if (s.is_selected)
			AddSubAction(new ActionModelSurfaceInvert(i), m);
}

ActionModelInvertSelection::~ActionModelInvertSelection()
{
}
