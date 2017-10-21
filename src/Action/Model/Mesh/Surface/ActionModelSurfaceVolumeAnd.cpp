/*
 * ActionModelSurfaceVolumeAnd.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceVolumeAnd.h"
#include "ActionModelDeleteSurface.h"
#include "../ActionModelPasteGeometry.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/Geometry/Geometry.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"


ActionModelSurfaceVolumeAnd::ActionModelSurfaceVolumeAnd()
{}

void *ActionModelSurfaceVolumeAnd::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int n = 0;
	for (ModelSurface &s: m->surface)
		if ((s.is_selected) && (s.is_closed))
			n ++;
	if (n == 0)
		throw ActionException("no closed surfaces selected");

	msg_db_f("Subtract", 1);
	Array<Geometry> geos;
	for (int bi=m->surface.num-1; bi>=0; bi--){
		if (m->surface[bi].is_selected){
			for (int ai=m->surface.num-1; ai>=0; ai--){
				ModelSurface *a = &m->surface[ai];
				if ((a->view_stage >= ed->multi_view_3d->view_stage) && (!a->is_selected))
					SurfaceAnd(m, a, ai, &m->surface[bi], geos);
			}
		}
	}
	for (Geometry &g: geos)
		addSubAction(new ActionModelPasteGeometry(g, 0), m);

	ed->setMessage(format(_("%d geschlossene Fl&achen subtrahiert"), n));
	return NULL;
}

void surf2geo(ModelSurface *s, Geometry &g);

void ActionModelSurfaceVolumeAnd::SurfaceAnd(DataModel *m, ModelSurface *a, int ai, ModelSurface *b, Array<Geometry> &geos)
{
	msg_db_f("SurfSubtract", 0);
	Geometry ga, gb, gc;
	surf2geo(a, ga);
	surf2geo(b, gb);
	int status = GeometryAnd(ga, gb, gc);

	if (status == 1){
		geos.add(gc);
		addSubAction(new ActionModelDeleteSurface(ai), m);
	}
}
