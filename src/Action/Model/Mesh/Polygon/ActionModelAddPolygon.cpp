/*
 * ActionAddPolygon.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionModelAddPolygon.h"
#include "../../../../Data/Model/DataModel.h"
#include "../Surface/Helper/ActionModelAddEmptySurface.h"
#include "../Surface/Helper/ActionModelSurfaceAddPolygon.h"
#include "../Surface/Helper/ActionModelJoinSurfaces.h"
#include <assert.h>

ActionModelAddPolygon::ActionModelAddPolygon(Array<int> &_v, int _material, Array<vector> &_sv) :
	v(_v), sv(_sv)
{
	material = _material;
}


void *ActionModelAddPolygon::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	//assert(_a >= 0 && _b >= 0 && _c >= 0);
	//assert(_a != _b && _b != _c && _c != _a);

	// add triangle
	return addSubAction(new ActionModelSurfaceAddPolygon(v, material, sv), m);
}
