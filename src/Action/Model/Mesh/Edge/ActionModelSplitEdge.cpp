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
		if ((t.vertex[k] != e.vertex[0]) && (t.vertex[k] != e.vertex[1])){
			v[0] = t.vertex[k];
			v[1] = t.vertex[(k + 1) % 3];
			v[2] = t.vertex[(k + 2) % 3];
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
	assert(surface < m->surface.num);
	ModelSurface &s = m->surface[surface];
	assert(edge >= 0);
	assert(edge < s.edge.num);

	ModelEdge &e = s.edge[edge];
	assert(e.polygon[0] >= 0);


	// add vertex
	vector pos = m->vertex[e.vertex[0]].pos * (1 - factor) + m->vertex[e.vertex[1]].pos * factor;
	addSubAction(new ActionModelAddVertex(pos), m);
	int new_vertex = m->vertex.num - 1;


	// adjacent polygons
	ModelEdge ee = e;
	for (int i=ee.ref_count-1;i>=0;i--){
		int poly = ee.polygon[i];
		ModelPolygon &t = s.polygon[poly];

		vector isv[MATERIAL_MAX_TEXTURES];
		float f = (i == 0) ? factor : 1 - factor;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			isv[l] = t.side[ee.side[i]].skin_vertex[l] * (1 - f) + t.side[(ee.side[i] + 1) % t.side.num].skin_vertex[l] * f;

		addSubAction(new ActionModelPolygonAddVertex(surface, poly, ee.side[i], new_vertex, isv), m);
	}
	return NULL;
}
