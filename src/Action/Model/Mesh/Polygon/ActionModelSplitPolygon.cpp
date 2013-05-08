/*
 * ActionModelSplitPolygon.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ActionModelSplitPolygon.h"
#include "../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../Surface/Helper/ActionModelSurfaceAddPolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"

ActionModelSplitPolygon::ActionModelSplitPolygon(int _surface, int _polygon, const vector &_pos) :
	pos(_pos)
{
	surface = _surface;
	polygon = _polygon;
}

void *ActionModelSplitPolygon::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelSurface &s = m->Surface[surface];
	ModelPolygon &t = s.Polygon[polygon];

	// old triangle data
	ModelPolygon temp = t;

	// skin interpolation
	SkinGeneratorMulti sg;
	sg.init_polygon(m, t);

	// delete old triangle
	AddSubAction(new ActionModelSurfaceDeletePolygon(surface, polygon), m);

	// create new vertex
	AddSubAction(new ActionModelAddVertex(pos), m);
	int new_vertex = m->Vertex.num - 1;

	// create 3 new triangles
	for (int k=0;k<temp.Side.num;k++){
		Array<int> v;
		v.add(temp.Side[k].Vertex);
		v.add(temp.Side[(k+1)%temp.Side.num].Vertex);
		v.add(new_vertex);
		Array<vector> sv;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
			sv.add(temp.Side[k].SkinVertex[l]);
			sv.add(temp.Side[(k+1)%temp.Side.num].SkinVertex[l]);
			sv.add(sg.get(pos, l));
		}
		AddSubAction(new ActionModelSurfaceAddPolygon(surface, v, temp.Material, sv), m);
	}

	return NULL;
}
