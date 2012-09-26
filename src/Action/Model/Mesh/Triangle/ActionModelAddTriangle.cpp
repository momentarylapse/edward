/*
 * ActionAddTriangle.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionModelAddTriangle.h"
#include "../../../../Data/Model/DataModel.h"
#include "../Surface/Helper/ActionModelAddEmptySurface.h"
#include "../Surface/Helper/ActionModelSurfaceAddTriangle.h"
#include "../Surface/Helper/ActionModelJoinSurfaces.h"
#include <assert.h>

ActionModelAddTriangle::ActionModelAddTriangle(DataModel *m, Array<int> &_v, int _material, Array<vector> &_sv)
{
	//assert(_a >= 0 && _b >= 0 && _c >= 0);
	//assert(_a != _b && _b != _c && _c != _a);
	Set<int> surf;
	foreach(int v, _v){
		if (m->Vertex[v].Surface >= 0)
			surf.add(m->Vertex[v].Surface);
	}

	if (surf.num == 0){
		// new surface
		AddSubAction(new ActionModelAddEmptySurface(), m);
		surf_no = m->Surface.num - 1;
	}else{
		// main surface?
		surf_no = surf[0];

		// join other surfaces into surf_no
		for (int i=surf.num-1;i>0;i--)
			AddSubAction(new ActionModelJoinSurfaces(surf_no, surf[i]), m);
	}

	// add triangle
	AddSubAction(new ActionModelSurfaceAddTriangle(surf_no, _v, _material, _sv), m);
}

ActionModelAddTriangle::~ActionModelAddTriangle()
{
}


void *ActionModelAddTriangle::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	return &m->Surface[surf_no].Polygon.back();
}
