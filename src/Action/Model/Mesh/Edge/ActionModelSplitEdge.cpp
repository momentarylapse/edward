/*
 * ActionModelSplitEdge.cpp
 *
 *  Created on: 09.06.2012
 *      Author: michi
 */

#include "ActionModelSplitEdge.h"
#include "../Polygon/Helper/ActionModelPolygonAddVertex.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

#if 0
inline int tria_sort_vert_by_edge(const ModelPolygon &t, const ModelEdge &e, int v[3])
{
	for (int k=0;k<3;k++)
		if ((t.Vertex[k] != e.Vertex[0]) && (t.Vertex[k] != e.Vertex[1])){
			v[0] = t.Vertex[k];
			v[1] = t.Vertex[(k + 1) % 3];
			v[2] = t.Vertex[(k + 2) % 3];
			return k;
		}
	assert(0 && "funny triangle");
	return -1;
}
#endif

ActionModelSplitEdge::ActionModelSplitEdge(int _surface, int _edge, float _factor)
{
	surface = _surface;
	edge = _edge;
	factor = _factor;
}

void *ActionModelSplitEdge::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(surface >= 0);
	assert(surface < m->Surface.num);
	ModelSurface &s = m->Surface[surface];
	assert(edge >= 0);
	assert(edge < s.Edge.num);

	ModelEdge &e = s.Edge[edge];
	assert(e.Polygon[0] >= 0);


	// add vertex
	vector pos = m->Vertex[e.Vertex[0]].pos * (1 - factor) + m->Vertex[e.Vertex[1]].pos * factor;
	AddSubAction(new ActionModelAddVertex(pos), m);
	int new_vertex = m->Vertex.num - 1;


	// adjacent polygons
	ModelEdge ee = e;
	for (int i=ee.RefCount-1;i>=0;i--){
		int poly = ee.Polygon[i];
		ModelPolygon &t = s.Polygon[poly];

		vector isv[MATERIAL_MAX_TEXTURES];
		float f = (i == 0) ? factor : 1 - factor;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			isv[l] = t.Side[ee.Side[i]].SkinVertex[l] * (1 - f) + t.Side[(ee.Side[i] + 1) % t.Side.num].SkinVertex[l] * f;

		AddSubAction(new ActionModelPolygonAddVertex(surface, poly, ee.Side[i], new_vertex, isv), m);
	}
	return NULL;
}
