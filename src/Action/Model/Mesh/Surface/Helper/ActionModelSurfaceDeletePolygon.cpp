/*
 * ActionModelSurfaceDeletePolygon.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceDeletePolygon.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../../../../Data/Model/ModelMesh.h"
#include "../../../../../Data/Model/ModelPolygon.h"

// might create an empty surface -> don't use alone!

ActionModelSurfaceDeletePolygon::ActionModelSurfaceDeletePolygon(int _index) {
	index = _index;
}

void ActionModelSurfaceDeletePolygon::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->mesh->_addPolygon(vertex, material, skin, index);
}



void *ActionModelSurfaceDeletePolygon::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelPolygon &t = m->mesh->polygon[index];

	// save old data
	material = t.material;
	vertex = t.getVertices();
	skin = t.getSkinVertices();

	// erase
	m->mesh->_removePolygon(index);

	return NULL;
}


