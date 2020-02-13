/*
 * ActionModelSurfaceAddPolygon.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAddPolygon.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../../../../Data/Model/ModelMesh.h"
#include "../../../../../Data/Model/ModelPolygon.h"

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
		m->mesh->_removePolygon(index);
	else
		m->mesh->_removePolygon(m->mesh->polygon.num -1);
}



void *ActionModelSurfaceAddPolygon::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->mesh->_addPolygon(v, material, sv, index);

	if (index >= 0)
		return &m->mesh->polygon[index];
	return &m->mesh->polygon.back();
}


