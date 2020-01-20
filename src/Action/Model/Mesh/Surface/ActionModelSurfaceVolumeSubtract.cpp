/*
 * ActionModelSurfaceVolumeSubtract.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceVolumeSubtract.h"
#include "ActionModelDeleteSurface.h"
#include "../ActionModelPasteGeometry.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/Geometry/Geometry.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"


ActionModelSurfaceVolumeSubtract::ActionModelSurfaceVolumeSubtract()
{}

void *ActionModelSurfaceVolumeSubtract::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int n = 0;
	for (ModelSurface &s: m->surface)
		if ((s.is_selected) && (s.is_closed))
			n ++;
	if (n == 0)
		throw ActionException("no closed surfaces selected");

	Array<Geometry> geos;
	for (int bi=m->surface.num-1; bi>=0; bi--){
		if (m->surface[bi].is_selected){
			for (int ai=m->surface.num-1; ai>=0; ai--){
				ModelSurface *a = &m->surface[ai];
				if ((a->view_stage >= ed->multi_view_3d->view_stage) && (!a->is_selected))
					SurfaceSubtract(m, a, ai, &m->surface[bi], geos);
			}
		}
	}
	for (Geometry &g: geos)
		addSubAction(new ActionModelPasteGeometry(g, 0), m);

	ed->set_message(format(_("%d closed surfaces subtracted"), n));
	return NULL;
}

void surf2geo(ModelSurface *s, Geometry &g)
{
	g.clear();
	for (int v: s->vertex)
		g.vertex.add(s->model->vertex[v]);
	for (ModelPolygon &p: s->polygon){
		ModelPolygon pp = p;
		for (int i=0;i<p.side.num;i++)
			pp.side[i].vertex = s->vertex.find(p.side[i].vertex);
		g.polygon.add(pp);
	}
}

void ActionModelSurfaceVolumeSubtract::SurfaceSubtract(DataModel *m, ModelSurface *a, int ai, ModelSurface *b, Array<Geometry> &geos)
{
	Geometry ga, gb, gc;
	surf2geo(a, ga);
	surf2geo(b, gb);
	int status = GeometrySubtract(ga, gb, gc);

	if (status == 1){
		geos.add(gc);
		addSubAction(new ActionModelDeleteSurface(ai), m);
	}
}
