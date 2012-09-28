/*
 * ActionModelInvertSelection.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelInvertSelection.h"
#include "ActionModelSurfaceInvert.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelInvertSelection::ActionModelInvertSelection()
{
}

void *ActionModelInvertSelection::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(ModelSurface &s, m->Surface, i)
		if (s.is_selected)
			AddSubAction(new ActionModelSurfaceInvert(i), m);
}
