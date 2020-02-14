/*
 * ActionModelPolygonRemoveVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelPolygonRemoveVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../../../../Data/Model/ModelMesh.h"
#include "../../../../../Data/Model/ModelPolygon.h"
#include "../../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../../Surface/Helper/ActionModelSurfaceAddPolygon.h"

ActionModelPolygonRemoveVertex::ActionModelPolygonRemoveVertex(int _poly, int _side) {
	poly = _poly;
	side = _side;
}

void *ActionModelPolygonRemoveVertex::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	auto &t = m->edit_mesh->polygon[poly];


	// save old polygon data
	Array<int> v = t.getVertices();
	Array<vector> _sv = t.getSkinVertices();
	int material = t.material;

	// remove vertex
	v.erase(side);
	for (int l=MATERIAL_MAX_TEXTURES-1;l>=0;l--)
		_sv.erase(side + l * t.side.num);

	// delete
	addSubAction(new ActionModelSurfaceDeletePolygon(poly), m);

	// recreate
	if (v.num > 2)
		addSubAction(new ActionModelSurfaceAddPolygon(v, material, _sv, poly), m);

	return NULL;
}

