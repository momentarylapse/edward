/*
 * ActionModelDeleteSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteSelection.h"
#include "Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Surface/Helper/ActionModelDeleteEmptySurface.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/ModelMesh.h"
#include "../../../Data/Model/ModelPolygon.h"
#include "../../../Data/Model/ModelSelection.h"

ActionModelDeleteSelection::ActionModelDeleteSelection(const ModelSelection &s, bool _greedy) : sel(s) {
	greedy = _greedy;
}

void *ActionModelDeleteSelection::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	Set<int> polys;

	if (greedy) {
		foreachib(ModelPolygon &t, m->edit_mesh->polygon, ti) {
			bool del = false;
			for (int k=0; k<t.side.num; k++)
				if (sel.vertex.contains(t.side[k].vertex))
					polys.add(ti);
		}
	} else {
		polys = sel.polygon;
	}

	foreachb (int i, polys)
		addSubAction(new ActionModelSurfaceDeletePolygon(i), m);

	foreachb (int i, sel.vertex)
		if (m->edit_mesh->vertex[i].ref_count == 0) {
			addSubAction(new ActionModelDeleteUnusedVertex(i), m);
		}

	return NULL;
}
