/*
 * ActionModelBevelVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelBevelVertices.h"
#include "../Triangle/ActionModelSplitEdge.h"
#include "../Triangle/ActionModelAddTrianglesByOutline.h"
#include "../Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

Array<int> get_boundary_loop(ModelSurface &s, int v0)
{
	Array<int> loop;
	int last = v0;
	bool found = true;
	while(found){
		found = false;
		foreach(ModelEdge &e, s.Edge)
			if (e.RefCount == 1)
				if (e.Vertex[0] == last){
					last = e.Vertex[1];
					loop.add(last);
					if (last == v0)
						return loop;
					found = true;
					break;
				}
	}
}

void ActionModelBevelVertices::BevelVertex(DataModel *m, float length, int vi)
{
	int surface = m->Vertex[vi].Surface;
	if (surface < 0)
		return;
	ModelSurface &s = m->Surface[surface];

	float epsilon = length * 0.001f;
	bool closed = true;

	// restrict length
	foreachib(ModelEdge &e, s.Edge, ei)
		for (int k=0;k<2;k++)
			if (e.Vertex[k] == vi){
				length = min(length, (m->Vertex[e.Vertex[(k+1)%2]].pos - m->Vertex[e.Vertex[k]].pos).length());
				if (e.RefCount < 2)
					closed = false;
			}

	// split the edges
	bool split = true;
	while(split){
		split = false;
		foreachib(ModelEdge &e, s.Edge, ei){
			for (int k=0;k<2;k++)
				if (e.Vertex[k] == vi){;
					vector dir = m->Vertex[e.Vertex[(k+1)%2]].pos - m->Vertex[e.Vertex[k]].pos;
					float edge_length = dir.length();
					if (edge_length < length + epsilon)
						continue;
					dir.normalize();

					vector pos = m->Vertex[e.Vertex[k]].pos + dir * length;

					AddSubAction(new ActionModelSplitEdge(m, surface, ei, pos), m);
					split = true;
					break;
			}
			if (split)
				break;
		}
	}

	// remove triangles
	int vert0 = -1;
	foreachib(ModelTriangle &t, s.Triangle, ti)
		for (int k=0;k<3;k++)
			if (t.Vertex[k] == vi){
				vert0 = t.Vertex[(k + 1) % 3];
				AddSubAction(new ActionModelSurfaceDeleteTriangle(surface, ti), m);
				_foreach_it_.update(); // TODO
				break;
			}

	// close hole
	if ((closed) && (vert0 >= 0)){
		Array<int> loop = get_boundary_loop(s, vert0);
		loop.reverse();
		msg_write(ia2s(loop));
		AddSubAction(new ActionModelAddTrianglesByOutline(loop, m), m);
	}
}

ActionModelBevelVertices::ActionModelBevelVertices(DataModel *m, float length)
{
	for (int i=m->Vertex.num-1;i>=0;i--)
		if (m->Vertex[i].is_selected)
			BevelVertex(m, fabs(length), i);
}

ActionModelBevelVertices::~ActionModelBevelVertices()
{
}

