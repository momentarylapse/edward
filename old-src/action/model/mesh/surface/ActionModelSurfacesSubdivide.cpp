/*
 * ActionModelSurfacesSubdivide.cpp
 *
 *  Created on: 05.01.2013
 *      Author: michi
 */

#include "ActionModelSurfacesSubdivide.h"
#include "helper/ActionModelSurfaceDeletePolygon.h"
#include "helper/ActionModelSurfaceAddPolygon.h"
#include "../vertex/ActionModelAddVertex.h"
#include "../vertex/helper/ActionModelMoveVertex.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include "../../../../data/model/ModelSelection.h"


ActionModelSurfacesSubdivide::ActionModelSurfacesSubdivide(const ModelSelection &s) :
	sel(s)
{
}

void *ActionModelSurfacesSubdivide::compose(Data *d) {
	auto mod = dynamic_cast<DataModel*>(d);
	auto m = mod->edit_mesh;
	if (!sel.consistent_surfaces(m))
		throw ActionException("surface selection inconsistent");

	// new polygon vertices
	int nv_p0 = m->vertex.num;
	Array<vec3> new_poly_vert;
	for (int ip: sel.polygon) {
		auto &p = m->polygon[ip];
		vec3 pos = v_0;
		for (int k=0; k<p.side.num; k++)
			pos += m->vertex[p.side[k].vertex].pos;
		pos /= p.side.num;
		new_poly_vert.add(pos);
		addSubAction(new ActionModelAddVertex(pos), mod);
	}

	// new edge vertices
	Array<vec3> old_edge_vert;
	int nv_e0 = m->vertex.num;
	for (auto &e: m->edge) {
		if (!sel.has(e))
			continue;
		vec3 pos = m->vertex[e.vertex[0]].pos + m->vertex[e.vertex[1]].pos;
		old_edge_vert.add(pos / 2);
		for (int k=0; k<e.ref_count; k++)
			pos += new_poly_vert[e.polygon[k]];
		pos /= 2 + e.ref_count;
		addSubAction(new ActionModelAddVertex(pos), mod);
	}

	// move old vertices
	for (int v: sel.vertex){
		vec3 F = v_0, R = v_0, P = m->vertex[v].pos;
		int n = 0;
		foreachi(ModelPolygon &p, m->polygon, i)
			for (int k=0; k<p.side.num; k++)
				if (p.side[k].vertex == v){
					F += new_poly_vert[i];
					R += old_edge_vert[p.side[k].edge];
					n ++;
				}
		R /= n;
		F /= n;
		addSubAction(new ActionModelMoveVertex(v, (F + 2*R + (n-3) * P) / n), mod);
	}

	// subdivide polygons
	Array<Array<int> > nv;
	Array<Array<vec3> > nsv;
	Array<int> nmat;
	for (int i: sel.polygon) {
		auto &p = m->polygon[i];
		Array<int> v = p.get_vertices();
		Array<vec3> sv = p.get_skin_vertices();
		int mat = p.material;

		// average skin vertex
		vec3 sv_med[MATERIAL_MAX_TEXTURES];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
			sv_med[l] = v_0;
			for (int k=0; k<p.side.num; k++)
				sv_med[l] += p.side[k].skin_vertex[l];
			sv_med[l] /= p.side.num;
		}

		for (int k=0; k<p.side.num; k++){
			nmat.add(mat);
			Array<int> vv;
			vv.add(v[k]);
			vv.add(nv_e0 + p.side[k].edge);
			vv.add(nv_p0 + i);
			vv.add(nv_e0 + p.side[(k + p.side.num - 1) % p.side.num].edge);
			nv.add(vv);
			Array<vec3> svv;
			for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
				svv.add(p.side[k].skin_vertex[l]);
				svv.add((p.side[k].skin_vertex[l] + p.side[(k+1)%p.side.num].skin_vertex[l]) / 2);
				svv.add(sv_med[l]);
				svv.add((p.side[k].skin_vertex[l] + p.side[(k+p.side.num-1)%p.side.num].skin_vertex[l]) / 2);
			}
			nsv.add(svv);
		}
	}

	for (int i=sel.polygon.num-1; i>=0; i--)
		addSubAction(new ActionModelSurfaceDeletePolygon(sel.polygon[i]), mod);

	for (int i=0; i<nv.num; i++)
		addSubAction(new ActionModelSurfaceAddPolygon(nv[i], nmat[i], nsv[i]), mod);


	return NULL;
}

