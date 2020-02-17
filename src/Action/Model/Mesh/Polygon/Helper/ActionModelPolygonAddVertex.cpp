/*
 * ActionModelPolygonAddVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelPolygonAddVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../../../../Data/Model/ModelMesh.h"
#include "../../../../../Data/Model/ModelPolygon.h"
#include "../../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../../Surface/Helper/ActionModelSurfaceAddPolygon.h"

ActionModelPolygonAddVertex::ActionModelPolygonAddVertex(int _poly, int _side, int _vertex, const vector* _sv) {
	poly = _poly;
	side = _side;
	vertex = _vertex;
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		sv[i] = _sv[i];
}

void* ActionModelPolygonAddVertex::compose(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelPolygon &t = m->edit_mesh->polygon[poly];


	// save old polygon data
	Array<int> v = t.get_vertices();
	Array<vector> _sv = t.get_skin_vertices();
	int material = t.material;

	// insert vertex
	v.insert(vertex, side + 1);
	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		_sv.insert(sv[l], side + 1 + l * t.side.num);

	// delete
	addSubAction(new ActionModelSurfaceDeletePolygon(poly), m);

	// recreate
	addSubAction(new ActionModelSurfaceAddPolygon(v, material, _sv, poly), m);

	return NULL;
}


