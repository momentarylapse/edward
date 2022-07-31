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
#include "../../../../Mode/Model/Mesh/ModeModelMesh.h"
#include <assert.h>

void ActionModelBevelVertices::BevelVertex(DataModel *m, float length, int vi)
{
	float epsilon = length * 0.0001f;
	bool closed = true;

	// restrict length
	foreachib(ModelEdge &e, m->edit_mesh->edge, ei)
		for (int k=0;k<2;k++)
			if (e.vertex[k] == vi){
				length = min(length, (m->edit_mesh->vertex[e.vertex[(k+1)%2]].pos - m->edit_mesh->vertex[e.vertex[k]].pos).length());
				if (e.ref_count < 2)
					closed = false;
			}

	int n_vert = m->edit_mesh->vertex.num;

	// split the edges
	bool split = true;
	while(split){
		split = false;
		foreachib(ModelEdge &e, m->edit_mesh->edge, ei){
			for (int k=0;k<2;k++)
				if (e.vertex[k] == vi){

					// don't split newly split edges!
					if (e.vertex[1-k] >= n_vert)
						continue;

					vec3 dir = m->edit_mesh->vertex[e.vertex[1-k]].pos - m->edit_mesh->vertex[e.vertex[k]].pos;
					float edge_length = dir.length();
					if (edge_length < length + epsilon)
						continue;

					// split
					float f = length/edge_length;
					if (k > 0)
						f = 1 - f;
					addSubAction(new ActionModelSplitEdge(ei, f), m);
					split = true;
					break;
				}
			if (split)
				break;
		}
	}

	// nothing done...
	if (n_vert == m->edit_mesh->vertex.num)
		return;


	// remove vi from polygons
	foreachi(ModelPolygon &t, m->edit_mesh->polygon, i)
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].vertex == vi){
				addSubAction(new ActionModelPolygonRemoveVertex(i, k), m);
				_foreach_it_.update(); // TODO
				break;
			}


	// close hole
	if (closed){
		Array<int> loop = m->edit_mesh->get_boundary_loop(m->edit_mesh->vertex.num - 1);
		loop.reverse();
		addSubAction(new ActionModelAddPolygonAutoSkin(loop, mode_model_mesh->current_material), m);
	}

	// delete vertex
	addSubAction(new ActionModelDeleteUnusedVertex(vi), m);
}

ActionModelBevelVertices::ActionModelBevelVertices(float _length)
{
	length = _length;
}

void *ActionModelBevelVertices::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	for (int i=m->edit_mesh->vertex.num-1;i>=0;i--)
		if (m->edit_mesh->vertex[i].is_selected)
			BevelVertex(m, fabs(length), i);
	return NULL;
}

