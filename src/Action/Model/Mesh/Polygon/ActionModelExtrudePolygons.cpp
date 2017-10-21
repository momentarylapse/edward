/*
 * ActionModelExtrudePolygons.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelExtrudePolygons.h"
#include "ActionModelAddPolygonSingleTexture.h"
#include "../Surface/Helper/ActionModelSurfaceRelinkPolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Vertex/Helper/ActionModelMoveVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Mode/Model/Mesh/ModeModelMesh.h"
#include <assert.h>

ActionModelExtrudePolygons::ActionModelExtrudePolygons(float _offset)
{
	offset = _offset;
}

void *ActionModelExtrudePolygons::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(ModelSurface &s, m->surface, si)
		ExtrudeSurface(s, si, m);

	return NULL;
}


void ActionModelExtrudePolygons::ExtrudeSurface(ModelSurface &s, int surface, DataModel *m)
{
	Set<int> sel_poly, sel_vert;
	foreachi(ModelPolygon &t, s.polygon, ti)
		if (t.is_selected){
			sel_poly.add(ti);
			for (int k=0;k<t.side.num;k++)
				sel_vert.add(t.side[k].vertex);
		}
	if (sel_poly.num == 0)
		return;

	// find boundary
	Set<int> boundary;
	for (ModelEdge &e: s.edge){
		int n_sel = 0;
		for (int k=0;k<e.ref_count;k++)
			if (s.polygon[e.polygon[k]].is_selected)
				n_sel ++;
		if (n_sel == 1){
			boundary.add(e.vertex[0]);
			boundary.add(e.vertex[1]);
		}
	}

	// copy boundary vertices
	Array<int> new_vert;
	for (int v: boundary){
		addSubAction(new ActionModelAddVertex(m->vertex[v].pos), m);
		new_vert.add(m->vertex.num - 1);
		//_foreach_it_.update(); // TODO
	}

	// move selected polygons
	for (int v: sel_vert){
		vector dir = v_0;
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected)
				for (int k=0;k<t.side.num;k++)
					if (v == t.side[k].vertex)
						dir += t.temp_normal;
		dir.normalize();
		addSubAction(new ActionModelMoveVertex(v, m->vertex[v].pos + dir * offset), m);
	}

	// re-link outer (=unselected) boundary polygons
	foreachi(ModelPolygon &t, s.polygon, ti)
		if (!t.is_selected){
			Array<int> v;
			bool on_boundary = false;
			for (int k=0;k<t.side.num;k++){
				v.add(t.side[k].vertex);
				int n = boundary.find(v[k]);
				if (n >= 0){
					on_boundary = true;
					v[k] = new_vert[n];
				}
			}
			if (on_boundary){
				addSubAction(new ActionModelSurfaceRelinkPolygon(surface, ti, v), m);
				_foreach_it_.update(); // TODO
			}
		}

	// fill "sides" of the extrusion
	foreachb(ModelEdge &e, s.edge)
		if (e.ref_count == 1){
			int n0 = boundary.find(e.vertex[0]);
			int n1 = boundary.find(e.vertex[1]);
			if ((n0 >= 0) and (n1 >= 0)){
				Array<int> v;
				v.add(e.vertex[1]);
				v.add(e.vertex[0]);
				v.add(new_vert[n0]);
				v.add(new_vert[n1]);
				Array<vector> sv;
				sv.add(vector(1,0,0));
				sv.add(vector(0,0,0));
				sv.add(vector(0,1,0));
				sv.add(vector(1,1,0));
				addSubAction(new ActionModelAddPolygonSingleTexture(v, mode_model_mesh->current_material, sv), m);
				_foreach_it_.update(); // TODO
			}
		}
}
