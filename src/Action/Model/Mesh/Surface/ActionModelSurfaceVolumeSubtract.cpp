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
#include "../../../../Data/Model/Geometry/ModelGeometry.h"
#include "../../../../Edward.h"


ActionModelSurfaceVolumeSubtract::ActionModelSurfaceVolumeSubtract()
{}

void *ActionModelSurfaceVolumeSubtract::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int n = 0;
	foreach(ModelSurface &s, m->Surface)
		if ((s.is_selected) && (s.IsClosed))
			n ++;
	if (n == 0)
		throw ActionException("no closed surfaces selected");

	msg_db_f("Subtract", 1);
	Array<ModelGeometry> geos;
	for (int bi=m->Surface.num-1; bi>=0; bi--){
		if (m->Surface[bi].is_selected){
			for (int ai=m->Surface.num-1; ai>=0; ai--){
				ModelSurface *a = &m->Surface[ai];
				if ((a->view_stage >= ed->multi_view_3d->view_stage) && (!a->is_selected))
					SurfaceSubtract(m, a, ai, &m->Surface[bi], geos);
			}
		}
	}
	foreach(ModelGeometry &g, geos)
		AddSubAction(new ActionModelPasteGeometry(g, 0), m);

	ed->SetMessage(format(_("%d geschlossene Fl&achen subtrahiert"), n));
	return NULL;
}

void surf2geo(ModelSurface *s, ModelGeometry &g)
{
	g.clear();
	foreach(int v, s->Vertex)
		g.Vertex.add(s->model->Vertex[v]);
	foreach(ModelPolygon &p, s->Polygon){
		ModelPolygon pp = p;
		for (int i=0;i<p.Side.num;i++)
			pp.Side[i].Vertex = s->Vertex.find(p.Side[i].Vertex);
		g.Polygon.add(pp);
	}
}

void ActionModelSurfaceVolumeSubtract::SurfaceSubtract(DataModel *m, ModelSurface *a, int ai, ModelSurface *b, Array<ModelGeometry> &geos)
{
	msg_db_f("SurfSubtract", 0);
	ModelGeometry ga, gb, gc;
	surf2geo(a, ga);
	surf2geo(b, gb);
	int status = ModelGeometrySubtract(ga, gb, gc);

	if (status == 1){
		geos.add(gc);
		AddSubAction(new ActionModelDeleteSurface(ai), m);
	}
}
