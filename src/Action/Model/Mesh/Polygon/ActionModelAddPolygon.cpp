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
	Set<int> surf;
	for (int vi: v){
		if (m->vertex[vi].surface >= 0)
			surf.add(m->vertex[vi].surface);
	}

	int surf_no;
	if (surf.num == 0){
		// new surface
		addSubAction(new ActionModelAddEmptySurface(), m);
		surf_no = m->surface.num - 1;
	}else{
		// main surface?
		surf_no = surf[0];

		// join other surfaces into surf_no
		for (int i=surf.num-1;i>0;i--)
			addSubAction(new ActionModelJoinSurfaces(surf_no, surf[i]), m);
	}

	// add triangle
	return addSubAction(new ActionModelSurfaceAddPolygon(surf_no, v, material, sv), m);
}
