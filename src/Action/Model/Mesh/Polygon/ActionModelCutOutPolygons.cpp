/*
 * ActionModelCutOutPolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ActionModelCutOutPolygons.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkPolygon.h"
#include "../Surface/Helper/ActionModelAddEmptySurface.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"
#include <assert.h>

ActionModelCutOutPolygons::ActionModelCutOutPolygons()
{
}

void *ActionModelCutOutPolygons::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	CutOutSurface(m);

	return NULL;
}


void ActionModelCutOutPolygons::CutOutSurface(DataModel *m)
{
	Set<int> sel_poly;
	foreachi(ModelPolygon &t, m->mesh->polygon, ti)
		if (t.is_selected)
			sel_poly.add(ti);
	if ((sel_poly.num == 0) or (sel_poly.num == m->mesh->polygon.num))
		return;

	// find boundary
	Set<int> boundary;
	for (ModelEdge &e: m->mesh->edge)
		if (e.ref_count == 2)
			if ((m->mesh->polygon[e.polygon[0]].is_selected != m->mesh->polygon[e.polygon[1]].is_selected)){
				boundary.add(e.vertex[0]);
				boundary.add(e.vertex[1]);
			}

	// copy boundary vertices
	Array<int> new_vert;
	for (int v: boundary){
		addSubAction(new ActionModelAddVertex(m->mesh->vertex[v].pos), m);
		new_vert.add(m->mesh->vertex.num - 1);
		//_foreach_it_.update(); // TODO
	}

	// move selected polygons
	foreachib(ModelPolygon &t, m->mesh->polygon, ti)
		if (t.is_selected){
			Array<int> v;
			for (int k=0;k<t.side.num;k++){
				v.add(t.side[k].vertex);
				// re-link if on boundary
				int n = boundary.find(v[k]);
				if (n >= 0)
					v[k] = new_vert[n];
			}
			addSubAction(new ActionModelSurfaceRelinkPolygon(ti, v), m);
			_foreach_it_.update(); // TODO
		}
}
