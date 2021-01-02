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
		m->edit_mesh->_remove_polygon(index);
	else
		m->edit_mesh->_remove_polygon(m->edit_mesh->polygon.num -1);
}



void *ActionModelSurfaceAddPolygon::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->edit_mesh->_add_polygon(v, material, sv, index);

	if (index >= 0)
		return &m->edit_mesh->polygon[index];
	return &m->edit_mesh->polygon.back();
}


