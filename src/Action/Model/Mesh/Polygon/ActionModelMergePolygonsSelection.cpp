/*
 * ActionModelMergePolygonsSelection.cpp
 *
 *  Created on: 05.01.2013
 *      Author: michi
 */

#include "ActionModelMergePolygonsSelection.h"
#include "../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../Surface/Helper/ActionModelSurfaceAddPolygon.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelMergePolygonsSelection::ActionModelMergePolygonsSelection()
{
}

void *ActionModelMergePolygonsSelection::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(ModelSurface &s, m->surface, si)
		MergePolygonsInSurface(m, &s, si);
	return NULL;
}

int polygons_count_shared_vertices(ModelPolygon &a, ModelPolygon &b)
{
	int n = 0;
	for (int i=0; i<a.side.num; i++)
		for (int j=0; j<b.side.num; j++)
			if (a.side[i].vertex == b.side[j].vertex)
				n ++;
	return n;
}

void ActionModelMergePolygonsSelection::MergePolygonsInSurface(DataModel *m, ModelSurface *s, int surface)
{
	int num_old_poly = s->polygon.num;
	bool found;
	do{
		found = false;

		foreachi(ModelEdge &e, s->edge, ei){
			if (e.ref_count < 2)
				continue;
			ModelPolygon &p0 = s->polygon[e.polygon[0]];
			ModelPolygon &p1 = s->polygon[e.polygon[1]];
			if ((!p0.is_selected) && (e.polygon[0] < num_old_poly))
				continue;
			if ((!p1.is_selected) && (e.polygon[1] < num_old_poly))
				continue;

			if (p0.material != p1.material)
				continue;

			if (p0.normal_dirty)
				p0.temp_normal = p0.getNormal(m->vertex);
			if (p1.normal_dirty)
				p1.temp_normal = p1.getNormal(m->vertex);

			if (p0.temp_normal * p1.temp_normal < 0.98f)
				continue;

			if (polygons_count_shared_vertices(p0, p1) != 2)
				continue;

			MergePolygons(m, s, surface, ei);
			found = true;
			num_old_poly -= 2;
			break;
		}
	}while (found);
}

void loop_sides(ModelPolygon &p, int steps)
{
	ModelPolygon temp = p;
	temp.side.resize(p.side.num);
	for (int i=0;i<p.side.num;i++)
		p.side[i] = temp.side[(i+p.side.num*5+steps) % p.side.num];
}

void ActionModelMergePolygonsSelection::MergePolygons(DataModel *m, ModelSurface *s, int surface, int edge)
{
	ModelEdge e = s->edge[edge];
	//msg_write(format("merge %d %d", e.Vertex[0], e.Vertex[1]));
	ModelPolygon p0 = s->polygon[e.polygon[0]];
	ModelPolygon p1 = s->polygon[e.polygon[1]];

	/*msg_write(ia2s(p0.GetVertices()));
	msg_write(e.Side[0]);
	msg_write(ia2s(p1.GetVertices()));
	msg_write(e.Side[1]);*/

	// merge polygon data
	loop_sides(p0, e.side[0] + 2);
	loop_sides(p1, e.side[1] + 2);
	p0.side.pop();
	p1.side.pop();
	p0.side += p1.side;
	Array<int> v = p0.getVertices();
	Array<vector> sv = p0.getSkinVertices();

	//msg_write(ia2s(v));

	// delete old polygons
	addSubAction(new ActionModelSurfaceDeletePolygon(surface, max(e.polygon[0], e.polygon[1])), m);
	addSubAction(new ActionModelSurfaceDeletePolygon(surface, min(e.polygon[0], e.polygon[1])), m);

	// add merged
	addSubAction(new ActionModelSurfaceAddPolygon(surface, v, p0.material, sv), m);
}
