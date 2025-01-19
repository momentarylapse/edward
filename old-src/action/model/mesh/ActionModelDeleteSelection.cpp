/*
 * ActionModelDeleteSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteSelection.h"
#include "surface/helper/ActionModelSurfaceDeletePolygon.h"
#include "vertex/helper/ActionModelDeleteUnusedVertex.h"
#include "surface/helper/ActionModelDeleteEmptySurface.h"
#include "physical/ActionModelDeleteBall.h"
#include "physical/ActionModelDeleteCylinder.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/ModelPolygon.h"
#include "../../../data/model/ModelSelection.h"
#include "../../../lib/os/msg.h"

ActionModelDeleteSelection::ActionModelDeleteSelection(const ModelSelection &s, bool _greedy) : sel(s) {
	greedy = _greedy;
}

void *ActionModelDeleteSelection::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	base::set<int> polys;

	if (greedy) {
		foreachib(ModelPolygon &t, m->edit_mesh->polygon, ti) {
			for (int k=0; k<t.side.num; k++)
				if (sel.vertex.contains(t.side[k].vertex))
					polys.add(ti);
		}
	} else {
		polys = sel.polygon;
	}

	//msg_write("del polys: " + str(polys));
	//msg_write("del balls: " + str(sel.ball));
	foreachb (int i, polys)
		addSubAction(new ActionModelSurfaceDeletePolygon(i), m);

	foreachb (int i, sel.ball)
		addSubAction(new ActionModelDeleteBall(i), m);
	foreachb (int i, sel.cylinder)
		addSubAction(new ActionModelDeleteCylinder(i), m);


	foreachb (int i, sel.vertex)
		if (m->edit_mesh->vertex[i].ref_count == 0) {
			addSubAction(new ActionModelDeleteUnusedVertex(i), m);
		}

	return NULL;
}
