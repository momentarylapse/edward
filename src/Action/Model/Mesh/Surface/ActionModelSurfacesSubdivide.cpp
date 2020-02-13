/*
 * ActionModelSurfacesSubdivide.cpp
 *
 *  Created on: 05.01.2013
 *      Author: michi
 */

#include "ActionModelSurfacesSubdivide.h"
#include "Helper/ActionModelSurfaceDeletePolygon.h"
#include "Helper/ActionModelSurfaceAddPolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Vertex/Helper/ActionModelMoveVertex.h"
#include "../../../../Data/Model/DataModel.h"

bool selection_consistent_surfaces(const ModelSelectionState &s, DataModel *m);

ActionModelSurfacesSubdivide::ActionModelSurfacesSubdivide(const ModelSelectionState &s) :
	sel(s)
{
}

void *ActionModelSurfacesSubdivide::compose(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	if (!selection_consistent_surfaces(sel, m))
		throw ActionException("surface selection inconsistent");

	// new polygon vertices
	int nv_p0 = m->vertex.num;
	Array<vector> new_poly_vert;
	for (int ip: sel.polygon) {
		auto &p = m->polygon[ip];
		vector pos = v_0;
		for (int k=0; k<p.side.num; k++)
			pos += m->vertex[p.side[k].vertex].pos;
		pos /= p.side.num;
		new_poly_vert.add(pos);
		addSubAction(new ActionModelAddVertex(pos), m);
	}

	// new edge vertices
	Array<vector> old_edge_vert;
	int nv_e0 = m->vertex.num;
	for (int ie: sel.edge) {
		auto &e = m->edge[ie];
		vector pos = m->vertex[e.vertex[0]].pos + m->vertex[e.vertex[1]].pos;
		old_edge_vert.add(pos / 2);
		for (int k=0; k<e.ref_count; k++)
			pos += new_poly_vert[e.polygon[k]];
		pos /= 2 + e.ref_count;
		addSubAction(new ActionModelAddVertex(pos), m);
	}

	// move old vertices
	for (int v: sel.vertex){
		vector F = v_0, R = v_0, P = m->vertex[v].pos;
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
		addSubAction(new ActionModelMoveVertex(v, (F + 2*R + (n-3) * P) / n), m);
	}

	// subdivide polygons
	Array<Array<int> > nv;
	Array<Array<vector> > nsv;
	Array<int> nmat;
	for (int i: sel.polygon) {
		auto &p = m->polygon[i];
		Array<int> v = p.getVertices();
		Array<vector> sv = p.getSkinVertices();
		int mat = p.material;

		// average skin vertex
		vector sv_med[MATERIAL_MAX_TEXTURES];
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
			Array<vector> svv;
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
		addSubAction(new ActionModelSurfaceDeletePolygon(sel.polygon[i]), m);

	for (int i=0; i<nv.num; i++)
		addSubAction(new ActionModelSurfaceAddPolygon(nv[i], nmat[i], nsv[i]), m);


	return NULL;
}

