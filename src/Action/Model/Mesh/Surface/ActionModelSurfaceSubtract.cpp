/*
 * ActionModelSurfaceSubtract.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceSubtract.h"
#include "ActionModelDeleteSurface.h"
#include "../ActionModelPasteGeometry.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/Geometry/ModelGeometry.h"
#include "../../../../Edward.h"


ActionModelSurfaceSubtract::ActionModelSurfaceSubtract()
{}

void *ActionModelSurfaceSubtract::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int n = 0;
	foreach(ModelSurface &s, m->Surface)
		if ((s.is_selected) && (s.IsClosed))
			n ++;
	if (n == 0){
		throw ActionException("no closed surfaces selected");
		//ed->SetMessage(_("Keine geschlossene Fl&achen markiert"));
		//return;
	}

	msg_db_f("Subtract", 1);
	for (int bi=m->Surface.num-1; bi>=0; bi--){
		if (m->Surface[bi].is_selected){
			for (int ai=m->Surface.num-1; ai>=0; ai--){
				ModelSurface *a = &m->Surface[ai];
				if ((a->view_stage >= ed->multi_view_3d->view_stage) && (!a->is_selected))
					SurfaceSubtract(m, a, ai, &m->Surface[bi]);
			}
		}
	}
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

void ActionModelSurfaceSubtract::SurfaceSubtract(DataModel *m, ModelSurface *a, int ai, ModelSurface *b)
{
	msg_db_f("SurfSubtract", 0);
	ModelGeometry ga, gb, gc;
	surf2geo(a, ga);
	surf2geo(b, gb);
	int status = ModelGeometrySubtract(ga, gb, gc);

	if (status == 1){
		AddSubAction(new ActionModelPasteGeometry(gc, 0), m);
		AddSubAction(new ActionModelDeleteSurface(ai), m);
	}
}
