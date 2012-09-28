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

	// bary centric
	/*float f, g;
	GetBaryCentric(pos, m->Vertex[va].pos, m->Vertex[vb].pos, m->Vertex[vc].pos, f, g);
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		sv[3][l] = sv[0][l] * (1 - f - g) + sv[1][l] * f + sv[2][l] * g;*/

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
		for (int l=0;l<MODEL_MAX_TEXTURES;l++){
			sv.add(temp.Side[k].SkinVertex[l]);
			sv.add(temp.Side[(k+1)%temp.Side.num].SkinVertex[l]);
			sv.add(v_0);
		}
		AddSubAction(new ActionModelSurfaceAddPolygon(surface, v, temp.Material, sv), m);
	}

	return NULL;
}
