/*
 * ActionAddPolygon.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionModelAddPolygon.h"
#include "../../../../data/model/DataModel.h"
#include "../surface/helper/ActionModelAddEmptySurface.h"
#include "../surface/helper/ActionModelSurfaceAddPolygon.h"
#include "../surface/helper/ActionModelJoinSurfaces.h"
#include <assert.h>

ActionModelAddPolygon::ActionModelAddPolygon(Array<int> &_v, int _material, Array<vec3> &_sv) :
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
