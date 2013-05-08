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

ActionModelSurfacesSubdivide::ActionModelSurfacesSubdivide(const Set<int> &_surfaces) :
	surfaces(_surfaces)
{
}

void *ActionModelSurfacesSubdivide::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	for (int i=surfaces.num-1; i>=0; i--)
		SubdivideSurface(m, &m->Surface[surfaces[i]], surfaces[i]);
	return NULL;
}

// Catmull-Clark subdivision
void ActionModelSurfacesSubdivide::SubdivideSurface(DataModel *m, ModelSurface *s, int surface)
{
	// new polygon vertices
	int nv_p0 = m->Vertex.num;
	Array<vector> new_poly_vert;
	foreach(ModelPolygon &p, s->Polygon){
		vector pos = v_0;
		for (int k=0; k<p.Side.num; k++)
			pos += m->Vertex[p.Side[k].Vertex].pos;
		pos /= p.Side.num;
		new_poly_vert.add(pos);
		AddSubAction(new ActionModelAddVertex(pos), m);
	}

	// new edge vertices
	Array<vector> old_edge_vert;
	int nv_e0 = m->Vertex.num;
	foreach(ModelEdge &e, s->Edge){
		vector pos = m->Vertex[e.Vertex[0]].pos + m->Vertex[e.Vertex[1]].pos;
		old_edge_vert.add(pos / 2);
		for (int k=0; k<e.RefCount; k++)
			pos += new_poly_vert[e.Polygon[k]];
		pos /= 2 + e.RefCount;
		AddSubAction(new ActionModelAddVertex(pos), m);
	}

	// move old vertices
	foreach(int v, s->Vertex){
		vector F = v_0, R = v_0, P = m->Vertex[v].pos;
		int n = 0;
		foreachi(ModelPolygon &p, s->Polygon, i)
			for (int k=0; k<p.Side.num; k++)
				if (p.Side[k].Vertex == v){
					F += new_poly_vert[i];
					R += old_edge_vert[p.Side[k].Edge];
					n ++;
				}
		R /= n;
		F /= n;
		AddSubAction(new ActionModelMoveVertex(v, (F + 2*R + (n-3) * P) / n), m);
	}

	// subdivide polygons
	Array<Array<int> > nv;
	Array<Array<vector> > nsv;
	Array<int> nmat;
	foreachib(ModelPolygon &p, s->Polygon, i){
		Array<int> v = p.GetVertices();
		Array<vector> sv = p.GetSkinVertices();
		int mat = p.Material;

		// average skin vertex
		vector sv_med[MATERIAL_MAX_TEXTURES];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
			sv_med[l] = v_0;
			for (int k=0; k<p.Side.num; k++)
				sv_med[l] += p.Side[k].SkinVertex[l];
			sv_med[l] /= p.Side.num;
		}

		for (int k=0; k<p.Side.num; k++){
			nmat.add(mat);
			Array<int> vv;
			vv.add(v[k]);
			vv.add(nv_e0 + p.Side[k].Edge);
			vv.add(nv_p0 + i);
			vv.add(nv_e0 + p.Side[(k + p.Side.num - 1) % p.Side.num].Edge);
			nv.add(vv);
			Array<vector> svv;
			for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
				svv.add(p.Side[k].SkinVertex[l]);
				svv.add((p.Side[k].SkinVertex[l] + p.Side[(k+1)%p.Side.num].SkinVertex[l]) / 2);
				svv.add(sv_med[l]);
				svv.add((p.Side[k].SkinVertex[l] + p.Side[(k+p.Side.num-1)%p.Side.num].SkinVertex[l]) / 2);
			}
			nsv.add(svv);
		}
	}

	for (int i=s->Polygon.num-1; i>=0; i--)
		AddSubAction(new ActionModelSurfaceDeletePolygon(surface, i), m);

	for (int i=0; i<nv.num; i++)
		AddSubAction(new ActionModelSurfaceAddPolygon(surface, nv[i], nmat[i], nsv[i]), m);
}

