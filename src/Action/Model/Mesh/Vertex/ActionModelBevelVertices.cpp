/*
 * ActionModelBevelVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelBevelVertices.h"
#include "Helper/ActionModelDeleteUnusedVertex.h"
#include "../Edge/ActionModelSplitEdge.h"
#include "../Polygon/Helper/ActionModelPolygonRemoveVertex.h"
#include "../Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

void ActionModelBevelVertices::BevelVertex(DataModel *m, float length, int vi)
{
	int surface = m->Vertex[vi].Surface;
	if (surface < 0)
		return;
	ModelSurface &s = m->Surface[surface];

	float epsilon = length * 0.0001f;
	bool closed = true;

	// restrict length
	foreachib(ModelEdge &e, s.Edge, ei)
		for (int k=0;k<2;k++)
			if (e.Vertex[k] == vi){
				length = min(length, (m->Vertex[e.Vertex[(k+1)%2]].pos - m->Vertex[e.Vertex[k]].pos).length());
				if (e.RefCount < 2)
					closed = false;
			}

	int n_vert = m->Vertex.num;

	// split the edges
	bool split = true;
	while(split){
		split = false;
		foreachib(ModelEdge &e, s.Edge, ei){
			for (int k=0;k<2;k++)
				if (e.Vertex[k] == vi){

					// don't split newly split edges!
					if (e.Vertex[1-k] >= n_vert)
						continue;

					vector dir = m->Vertex[e.Vertex[1-k]].pos - m->Vertex[e.Vertex[k]].pos;
					float edge_length = dir.length();
					if (edge_length < length + epsilon)
						continue;

					// split
					float f = length/edge_length;
					if (k > 0)
						f = 1 - f;
					AddSubAction(new ActionModelSplitEdge(surface, ei, f), m);
					split = true;
					break;
				}
			if (split)
				break;
		}
	}

	// nothing done...
	if (n_vert == m->Vertex.num)
		return;


	// remove vi from polygons
	foreachi(ModelPolygon &t, s.Polygon, i)
		for (int k=0;k<t.Side.num;k++)
			if (t.Side[k].Vertex == vi){
				AddSubAction(new ActionModelPolygonRemoveVertex(surface, i, k), m);
				_foreach_it_.update(); // TODO
				break;
			}


	// close hole
	if (closed){
		Array<int> loop = s.GetBoundaryLoop(m->Vertex.num - 1);
		loop.reverse();
		AddSubAction(new ActionModelAddPolygonAutoSkin(loop), m);
	}

	// delete vertex
	AddSubAction(new ActionModelDeleteUnusedVertex(vi), m);
}

ActionModelBevelVertices::ActionModelBevelVertices(float _length)
{
	length = _length;
}

void *ActionModelBevelVertices::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	for (int i=m->Vertex.num-1;i>=0;i--)
		if (m->Vertex[i].is_selected)
			BevelVertex(m, fabs(length), i);
	return NULL;
}

