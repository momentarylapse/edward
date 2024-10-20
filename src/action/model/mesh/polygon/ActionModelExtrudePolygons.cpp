/*
 * ActionModelExtrudePolygons.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelExtrudePolygons.h"
#include "ActionModelAddPolygonSingleTexture.h"
#include "../surface/helper/ActionModelSurfaceRelinkPolygon.h"
#include "../vertex/ActionModelAddVertex.h"
#include "../vertex/helper/ActionModelMoveVertex.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include <assert.h>

ActionModelExtrudePolygons::ActionModelExtrudePolygons(float _offset, bool _independent, int _material) {
	offset = _offset;
	independent = _independent;
	material = _material;
}

void *ActionModelExtrudePolygons::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	if (independent) {
		extrude_surface_indep(m);
	} else {
		extrude_surface(m);
	}

	return NULL;
}


void ActionModelExtrudePolygons::extrude_surface(DataModel *m) {
	base::set<int> sel_poly, sel_vert;
	foreachi(auto &t, m->edit_mesh->polygon, ti)
		if (t.is_selected) {
			sel_poly.add(ti);
			for (int k=0;k<t.side.num;k++)
				sel_vert.add(t.side[k].vertex);
		}
	if (sel_poly.num == 0)
		return;

	// find boundary
	base::set<int> boundary;
	for (ModelEdge &e: m->edit_mesh->edge) {
		int n_sel = 0;
		for (int k=0;k<e.ref_count;k++)
			if (m->edit_mesh->polygon[e.polygon[k]].is_selected)
				n_sel ++;
		if (n_sel == 1)  {
			boundary.add(e.vertex[0]);
			boundary.add(e.vertex[1]);
		}
	}

	// copy boundary vertices
	Array<int> new_vert;
	for (int v: boundary) {
		addSubAction(new ActionModelAddVertex(m->edit_mesh->vertex[v].pos), m);
		new_vert.add(m->edit_mesh->vertex.num - 1);
	}

	// move selected polygons
	for (int v: sel_vert){
		vec3 dir = v_0;
		for (auto &t: m->edit_mesh->polygon)
			if (t.is_selected)
				for (int k=0;k<t.side.num;k++)
					if (v == t.side[k].vertex)
						dir += t.temp_normal;
		dir.normalize();
		addSubAction(new ActionModelMoveVertex(v, m->edit_mesh->vertex[v].pos + dir * offset), m);
	}

	// re-link outer (=unselected) boundary polygons
	foreachi(auto &t, m->edit_mesh->polygon, ti)
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
			if (on_boundary) {
				addSubAction(new ActionModelSurfaceRelinkPolygon(ti, v), m);
				_foreach_it_.update(); // TODO
			}
		}

	Array<int> sewing;

	// fill "sides" of the extrusion
	foreachb(auto &e, m->edit_mesh->edge)
		if (e.ref_count == 1) {
			int n0 = boundary.find(e.vertex[0]);
			int n1 = boundary.find(e.vertex[1]);
			if ((n0 >= 0) and (n1 >= 0)) {
				sewing.add(e.vertex[1]);
				sewing.add(e.vertex[0]);
				sewing.add(new_vert[n0]);
				sewing.add(new_vert[n1]);
			}
		}


	for (int i=0; i<sewing.num; i+=4) {
		Array<vec3> sv;
		sv.add(vec3(1,0,0));
		sv.add(vec3(0,0,0));
		sv.add(vec3(0,1,0));
		sv.add(vec3(1,1,0));
		addSubAction(new ActionModelAddPolygonSingleTexture(sewing.sub_ref(i, i+4), material, sv), m);
	}
}


void ActionModelExtrudePolygons::extrude_surface_indep(DataModel *m) {
	Array<int> sewing;

	for (int ti=0; ti<m->edit_mesh->polygon.num; ti++) {
		auto &t = m->edit_mesh->polygon[ti];
		if (!t.is_selected)
			continue;

		Array<int> sel_vert;
		for (int k=0;k<t.side.num;k++)
			sel_vert.add(t.side[k].vertex);

		vec3 n = t.temp_normal;


		// copy/move boundary vertices
		Array<int> new_vert;
		for (int v: sel_vert) {
			addSubAction(new ActionModelAddVertex(m->edit_mesh->vertex[v].pos + n*offset), m);
			new_vert.add(m->edit_mesh->vertex.num - 1);
		}

		// re-link polygon
		addSubAction(new ActionModelSurfaceRelinkPolygon(ti, new_vert), m);

		// fill "sides" of the extrusion
		for (int i=0; i<sel_vert.num; i++) {
			int ii = (i+1) % sel_vert.num;
			sewing.add(sel_vert[i]);
			sewing.add(sel_vert[ii]);
			sewing.add(new_vert[ii]);
			sewing.add(new_vert[i]);
		}
	}


	for (int i=0; i<sewing.num; i+=4) {
		Array<vec3> sv;
		sv.add(vec3(1,0,0));
		sv.add(vec3(0,0,0));
		sv.add(vec3(0,1,0));
		sv.add(vec3(1,1,0));
		addSubAction(new ActionModelAddPolygonSingleTexture(sewing.sub_ref(i, i+4), material, sv), m);
	}

}
