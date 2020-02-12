/*
 * ActionModelSurfaceAddPolygon.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAddPolygon.h"
#include "../../../../../Data/Model/DataModel.h"

// might create a "disjoint" surface -> don't use alone!

ActionModelSurfaceAddPolygon::ActionModelSurfaceAddPolygon(const Array<int> &_v, int _material, const Array<vector> &_sv, int _index) {
	index = _index;
	v = _v;
	material = _material;
	sv = _sv;
}

void ActionModelSurfaceAddPolygon::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	if (index >= 0)
		m->_removePolygon(index);
	else
		m->_removePolygon(m->polygon.num -1);
}



void *ActionModelSurfaceAddPolygon::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->_addPolygon(v, material, sv, index);

	if (index >= 0)
		return &m->polygon[index];
	return &m->polygon.back();
}


