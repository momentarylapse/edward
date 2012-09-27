/*
 * ActionModelCutOutPolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ActionModelCutOutPolygons.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkPolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelCutOutPolygons::ActionModelCutOutPolygons()
{
}

void *ActionModelCutOutPolygons::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(ModelSurface &s, m->Surface, si)
		CutOutSurface(s, si, m);

	return NULL;
}


void ActionModelCutOutPolygons::CutOutSurface(ModelSurface &s, int surface, DataModel *m)
{
	Set<int> sel_poly;
	foreachi(ModelPolygon &t, s.Polygon, ti)
		if (t.is_selected)
			sel_poly.add(ti);
	if ((sel_poly.num == 0) or (sel_poly.num == s.Polygon.num))
		return;

	// find boundary
	Set<int> boundary;
	foreach(ModelEdge &e, s.Edge)
		if (e.RefCount == 2)
			if ((s.Polygon[e.Polygon[0]].is_selected != s.Polygon[e.Polygon[1]].is_selected)){
				boundary.add(e.Vertex[0]);
				boundary.add(e.Vertex[1]);
			}

	// copy boundary vertices
	Array<int> new_vert;
	foreach(int v, boundary){
		AddSubAction(new ActionModelAddVertex(m->Vertex[v].pos), m);
		new_vert.add(m->Vertex.num - 1);
		_foreach_it_.update(); // TODO
	}

	// re-link outer (=unselected) boundary polygons
	foreachi(ModelPolygon &t, s.Polygon, ti)
		if (!t.is_selected){
			Array<int> v;
			bool on_boundary = false;
			for (int k=0;k<t.Side.num;k++){
				v.add(t.Side[k].Vertex);
				int n = boundary.find(v[k]);
				if (n >= 0){
					on_boundary = true;
					v[k] = new_vert[n];
				}
			}
			if (on_boundary){
				AddSubAction(new ActionModelSurfaceRelinkPolygon(surface, ti, v), m);
				_foreach_it_.update(); // TODO
			}
		}
}
