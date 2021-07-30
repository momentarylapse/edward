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

	Geometry gsel, gunsel;
	gsel.vertex = m->mesh->vertex;
	gunsel.vertex = m->mesh->vertex;
	for (auto &p: m->mesh->polygon)
		if (p.is_selected)
			gsel.polygon.add(p);
		else
			gunsel.polygon.add(p);

	if (gsel.polygon.num == 0)
		throw ActionException("no polygons selected");

	gsel.update_topology();
	gunsel.update_topology();
	gsel.remove_unused_vertices();
	gunsel.remove_unused_vertices();

	if (!gsel.is_closed())
		throw ActionException("selected surface is not closed");

	auto gunsel_components = gunsel.split_connected();
	Array<Geometry> gsub;
	bool changed = false;

	for (auto &g: gunsel_components) {

		Geometry gc;
		int status = GeometrySubtract(g, gsel, gc);
		changed |= (status == 1);
		if (status == 1) {
			gsub.add(gc);
		} else {
			gsub.add(g);
		}

	}

	if (changed or true) {
		auto sel = ModelSelection::all(m->mesh);
		addSubAction(new ActionModelDeleteSelection(sel, false), d);

		for (auto &g: gsub)
			addSubAction(new ActionModelPasteGeometry(g, 0), d);
		addSubAction(new ActionModelPasteGeometry(gsel, 0), d);
	}
	return nullptr;
}
