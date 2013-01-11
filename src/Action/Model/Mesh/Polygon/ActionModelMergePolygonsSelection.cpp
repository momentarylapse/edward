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
	foreachi(ModelSurface &s, m->Surface, si)
		MergePolygonsInSurface(m, &s, si);
	return NULL;
}

int polygons_count_shared_vertices(ModelPolygon &a, ModelPolygon &b)
{
	int n = 0;
	for (int i=0; i<a.Side.num; i++)
		for (int j=0; j<b.Side.num; j++)
			if (a.Side[i].Vertex == b.Side[j].Vertex)
				n ++;
	return n;
}

void ActionModelMergePolygonsSelection::MergePolygonsInSurface(DataModel *m, ModelSurface *s, int surface)
{
	int num_old_poly = s->Polygon.num;
	bool found;
	do{
		found = false;

		foreachi(ModelEdge &e, s->Edge, ei){
			if (e.RefCount < 2)
				continue;
			ModelPolygon &p0 = s->Polygon[e.Polygon[0]];
			ModelPolygon &p1 = s->Polygon[e.Polygon[1]];
			if ((!p0.is_selected) && (e.Polygon[0] < num_old_poly))
				continue;
			if ((!p1.is_selected) && (e.Polygon[1] < num_old_poly))
				continue;

			if (p0.Material != p1.Material)
				continue;

			if (p0.NormalDirty)
				p0.TempNormal = p0.GetNormal(m->Vertex);
			if (p1.NormalDirty)
				p1.TempNormal = p1.GetNormal(m->Vertex);

			if (p0.TempNormal * p1.TempNormal < 0.98f)
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
	temp.Side.resize(p.Side.num);
	for (int i=0;i<p.Side.num;i++)
		p.Side[i] = temp.Side[(i+p.Side.num*5+steps) % p.Side.num];
}

void ActionModelMergePolygonsSelection::MergePolygons(DataModel *m, ModelSurface *s, int surface, int edge)
{
	ModelEdge e = s->Edge[edge];
	//msg_write(format("merge %d %d", e.Vertex[0], e.Vertex[1]));
	ModelPolygon p0 = s->Polygon[e.Polygon[0]];
	ModelPolygon p1 = s->Polygon[e.Polygon[1]];

	/*msg_write(ia2s(p0.GetVertices()));
	msg_write(e.Side[0]);
	msg_write(ia2s(p1.GetVertices()));
	msg_write(e.Side[1]);*/

	// merge polygon data
	loop_sides(p0, e.Side[0] + 2);
	loop_sides(p1, e.Side[1] + 2);
	p0.Side.pop();
	p1.Side.pop();
	p0.Side += p1.Side;
	Array<int> v = p0.GetVertices();
	Array<vector> sv = p0.GetSkinVertices();

	//msg_write(ia2s(v));

	// delete old polygons
	AddSubAction(new ActionModelSurfaceDeletePolygon(surface, max(e.Polygon[0], e.Polygon[1])), m);
	AddSubAction(new ActionModelSurfaceDeletePolygon(surface, min(e.Polygon[0], e.Polygon[1])), m);

	// add merged
	AddSubAction(new ActionModelSurfaceAddPolygon(surface, v, p0.Material, sv), m);
}
