/*
 * ActionModelExtrudeTriangles.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelExtrudeTriangles.h"
//#include "../Surface/Helper/ActionModelSurfaceAddTriangle.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkTriangle.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Vertex/ActionModelMoveVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelExtrudeTriangles::ActionModelExtrudeTriangles(DataModel *data, float offset)
{
	vector dir = v_0;
	foreach(ModelSurface &s, data->Surface)
		foreach(ModelTriangle &t, s.Triangle)
			if (t.is_selected)
				dir += t.TempNormal;
	dir.normalize();

	vector dpos = dir * offset;
	ActionMultiView *a = new ActionModelMoveVertices(data, v_0);
	a->set_axis(e_x, e_y, e_z);
	a->set_param_and_notify(data, dpos);
	AddSubAction(a, data);

	foreachi(ModelSurface &s, data->Surface, si)
		ExtrudeSurface(s, si, data, dpos);
}

ActionModelExtrudeTriangles::~ActionModelExtrudeTriangles()
{
}


void ActionModelExtrudeTriangles::ExtrudeSurface(ModelSurface &s, int surface, DataModel *m, const vector &dpos)
{
	Set<int> sel;
	foreachi(ModelTriangle &t, s.Triangle, ti)
		if (t.is_selected)
			sel.add(ti);
	if ((sel.num == 0) or (sel.num == s.Triangle.num))
		return;

	// find boundary
	Set<int> boundary;
	foreach(ModelEdge &e, s.Edge)
		if (e.RefCount == 2)
			if ((s.Triangle[e.Triangle[0]].is_selected != s.Triangle[e.Triangle[1]].is_selected)){
				boundary.add(e.Vertex[0]);
				boundary.add(e.Vertex[1]);
			}

	// copy boundary vertices
	Array<int> new_vert;
	foreach(int v, boundary){
		AddSubAction(new ActionModelAddVertex(m->Vertex[v].pos - dpos), m);
		new_vert.add(m->Vertex.num - 1);
		_foreach_it_.update(); // TODO
	}

	// re-link boundary triangles
	foreachi(ModelTriangle &t, s.Triangle, ti)
		if (!t.is_selected){
			int v[3];
			bool on_boundary = false;
			for (int k=0;k<3;k++){
				v[k] = t.Vertex[k];
				int n = boundary.find(v[k]);
				if (n >= 0){
					on_boundary = true;
					v[k] = new_vert[n];
				}
			}
			if (on_boundary){
				AddSubAction(new ActionModelSurfaceRelinkTriangle(m, surface, ti, v[0], v[1], v[2]), m);
				_foreach_it_.update(); // TODO
			}
		}

	// fill "sides" of the extrusion
	foreachb(ModelEdge &e, s.Edge)
		if (e.RefCount == 1){
			int n0 = boundary.find(e.Vertex[0]);
			int n1 = boundary.find(e.Vertex[1]);
			if ((n0 >= 0) and (n1 >= 0)){
				int a = e.Vertex[1];
				int b = e.Vertex[0];
				int c = new_vert[n1];
				int d = new_vert[n0];
				AddSubAction(new ActionModelAddTriangleSingleTexture(m, a, b, c, m->CurrentMaterial, v_0, e_x, e_y), m);
				AddSubAction(new ActionModelAddTriangleSingleTexture(m, c, b, d, m->CurrentMaterial, e_y, e_x, e_x + e_y), m);
				_foreach_it_.update(); // TODO
			}
		}
}
