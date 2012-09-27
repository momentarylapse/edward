/*
 * ActionModelExtrudePolygons.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelExtrudePolygons.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkPolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Vertex/Helper/ActionModelMoveVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelExtrudePolygons::ActionModelExtrudePolygons(float _offset)
{
	offset = _offset;
}

void *ActionModelExtrudePolygons::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(ModelSurface &s, m->Surface, si)
		ExtrudeSurface(s, si, m);

	return NULL;
}


void ActionModelExtrudePolygons::ExtrudeSurface(ModelSurface &s, int surface, DataModel *m)
{
	Set<int> sel_poly, sel_vert;
	foreachi(ModelPolygon &t, s.Polygon, ti)
		if (t.is_selected){
			sel_poly.add(ti);
			for (int k=0;k<t.Side.num;k++)
				sel_vert.add(t.Side[k].Vertex);
		}
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

	// move selected polygons
	foreach(int v, sel_vert){
		vector dir = v_0;
		foreach(ModelPolygon &t, s.Polygon)
			if (t.is_selected)
				for (int k=0;k<t.Side.num;k++)
					if (v == t.Side[k].Vertex)
						dir += t.TempNormal;
		dir.normalize();
		AddSubAction(new ActionModelMoveVertex(v, m->Vertex[v].pos + dir * offset), m);
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

	// fill "sides" of the extrusion
	foreachb(ModelEdge &e, s.Edge)
		if (e.RefCount == 1){
			int n0 = boundary.find(e.Vertex[0]);
			int n1 = boundary.find(e.Vertex[1]);
			if ((n0 >= 0) and (n1 >= 0)){
				Array<int> v;
				v.add(e.Vertex[1]);
				v.add(e.Vertex[0]);
				v.add(new_vert[n0]);
				v.add(new_vert[n1]);
				Array<vector> sv;
				sv.add(vector(1,0,0));
				sv.add(vector(0,0,0));
				sv.add(vector(0,1,0));
				sv.add(vector(1,1,0));
				AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, m->CurrentMaterial, sv), m);
				_foreach_it_.update(); // TODO
			}
		}
}
