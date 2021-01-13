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
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"

ActionModelMergePolygonsSelection::ActionModelMergePolygonsSelection() {
}

void *ActionModelMergePolygonsSelection::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	MergePolygonsInSurface(m);
	return NULL;
}

int polygons_count_shared_vertices(ModelPolygon &a, ModelPolygon &b) {
	int n = 0;
	for (int i=0; i<a.side.num; i++)
		for (int j=0; j<b.side.num; j++)
			if (a.side[i].vertex == b.side[j].vertex)
				n ++;
	return n;
}

int count_poly_sel(DataModel *m) {
	int n = 0;
	for (auto &p: m->edit_mesh->polygon)
		if (p.is_selected)
			n ++;
	return n;
}

void ActionModelMergePolygonsSelection::MergePolygonsInSurface(DataModel *m) {
	int num_old_poly = m->edit_mesh->polygon.num;
	bool found;

	do {
		found = false;

		foreachi(ModelEdge &e, m->edit_mesh->edge, ei) {
			if (e.ref_count < 2)
				continue;
			ModelPolygon &p0 = m->edit_mesh->polygon[e.polygon[0]];
			ModelPolygon &p1 = m->edit_mesh->polygon[e.polygon[1]];
			if ((!p0.is_selected) and (e.polygon[0] < num_old_poly))
				continue;
			if ((!p1.is_selected) and (e.polygon[1] < num_old_poly))
				continue;

			if (p0.material != p1.material)
				continue;

			if (p0.normal_dirty)
				p0.temp_normal = p0.get_normal(m->edit_mesh->vertex);
			if (p1.normal_dirty)
				p1.temp_normal = p1.get_normal(m->edit_mesh->vertex);

			if (p0.temp_normal * p1.temp_normal < 0.99f)
				continue;

			if (polygons_count_shared_vertices(p0, p1) != 2)
				continue;

			MergePolygons(m, ei, num_old_poly);
			found = true;
			break;
		}
	} while (found);
}

void loop_sides(ModelPolygon &p, int steps) {
	ModelPolygon temp = p;
	temp.side.resize(p.side.num);
	for (int i=0;i<p.side.num;i++)
		p.side[i] = temp.side[(i+p.side.num*5+steps) % p.side.num];
}

void ActionModelMergePolygonsSelection::MergePolygons(DataModel *m, int edge, int &max_old_poly) {
	ModelEdge e = m->edit_mesh->edge[edge];
	//msg_write(format("merge  %d  %d", e.polygon[0], e.polygon[1]));
	ModelPolygon p0 = m->edit_mesh->polygon[e.polygon[0]];
	ModelPolygon p1 = m->edit_mesh->polygon[e.polygon[1]];

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
	Array<int> v = p0.get_vertices();
	Array<vector> sv = p0.get_skin_vertices();

	for (int k=0; k<2; k++)
		if (e.polygon[k] < max_old_poly)
			max_old_poly --;

	//msg_write(ia2s(v));

	// delete old polygons
	addSubAction(new ActionModelSurfaceDeletePolygon(max(e.polygon[0], e.polygon[1])), m);
	addSubAction(new ActionModelSurfaceDeletePolygon(min(e.polygon[0], e.polygon[1])), m);

	// add merged
	addSubAction(new ActionModelSurfaceAddPolygon(v, p0.material, sv), m);
}
