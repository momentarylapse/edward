/*
 * ActionModelSurfaceVolumeSubtract.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceVolumeSubtract.h"
#include "ActionModelDeleteSurface.h"
#include "../ActionModelPasteGeometry.h"
#include "../ActionModelDeleteSelection.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/Geometry/Geometry.h"
#include "../../../../Data/Model/ModelSelection.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"


ActionModelSurfaceVolumeSubtract::ActionModelSurfaceVolumeSubtract()
{}

void *ActionModelSurfaceVolumeSubtract::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	Geometry ga, gb, gc;
	ga.vertex = m->mesh->vertex;
	gb.vertex = m->mesh->vertex;
	for (auto &p: m->mesh->polygon)
		if (p.is_selected)
			gb.polygon.add(p);
		else
			ga.polygon.add(p);

	if (gb.polygon.num == 0)
		throw ActionException("no polygons selected");

	ga.update_topology();
	gb.update_topology();
	ga.remove_unused_vertices();
	gb.remove_unused_vertices();

	if (!gb.is_closed)
		throw ActionException("selected surface is not closed");

	int status = GeometrySubtract(ga, gb, gc);

	if (status == 1) {
		auto sel = ModelSelection::all(m->mesh);
		addSubAction(new ActionModelDeleteSelection(sel, false), d);

		addSubAction(new ActionModelPasteGeometry(gc, 0), d);
		addSubAction(new ActionModelPasteGeometry(gb, 0), d);
	}
	return NULL;
}
