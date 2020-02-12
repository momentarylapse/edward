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

ActionModelSplitPolygon::ActionModelSplitPolygon(int _polygon, const vector &_pos) :
	pos(_pos)
{
	polygon = _polygon;
}

void *ActionModelSplitPolygon::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelPolygon &t = m->polygon[polygon];

	// old triangle data
	ModelPolygon temp = t;

	// skin interpolation
	SkinGeneratorMulti sg;
	sg.init_polygon(m->vertex, t);

	// delete old triangle
	addSubAction(new ActionModelSurfaceDeletePolygon(polygon), m);

	// create new vertex
	addSubAction(new ActionModelAddVertex(pos), m);
	int new_vertex = m->vertex.num - 1;

	// create 3 new triangles
	for (int k=0;k<temp.side.num;k++){
		Array<int> v;
		v.add(temp.side[k].vertex);
		v.add(temp.side[(k+1)%temp.side.num].vertex);
		v.add(new_vertex);
		Array<vector> sv;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
			sv.add(temp.side[k].skin_vertex[l]);
			sv.add(temp.side[(k+1)%temp.side.num].skin_vertex[l]);
			sv.add(sg.get(pos, l));
		}
		addSubAction(new ActionModelSurfaceAddPolygon(v, temp.material, sv), m);
	}

	return NULL;
}
