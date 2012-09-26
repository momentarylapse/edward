/*
 * ActionModelPolygonAddVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelPolygonAddVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "../../Surface/Helper/ActionModelSurfaceAddTriangle.h"

ActionModelPolygonAddVertex::ActionModelPolygonAddVertex(int _surface,
		int _poly, int _side, int _vertex, const vector* _sv)
{
	surface = _surface;
	poly = _poly;
	side = _side;
	vertex = _vertex;
	for (int i=0;i<MODEL_MAX_TEXTURES;i++)
		sv[i] = _sv[i];
}

void* ActionModelPolygonAddVertex::compose(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface &s = m->Surface[surface];
	ModelPolygon &t = s.Polygon[poly];


	// save old polygon data
	Array<int> v;
	Array<vector> _sv;
	for (int k=0;k<t.Side.num;k++)
		v.add(t.Side[k].Vertex);
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<t.Side.num;k++)
			_sv.add(t.Side[k].SkinVertex[l]);
	int material = t.Material;

	// insert vertex
	v.insert(vertex, side + 1);
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		_sv.insert(sv[l], side + 1 + l * t.Side.num);

	// delete
	AddSubAction(new ActionModelSurfaceDeleteTriangle(surface, poly), m);

	// recreate
	AddSubAction(new ActionModelSurfaceAddTriangle(surface, v, material, _sv, poly), m);

	return NULL;
}


