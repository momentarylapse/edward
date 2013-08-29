/*
 * ActionModelBevelEdges.cpp
 *
 *  Created on: 06.01.2013
 *      Author: michi
 */

#include "ActionModelBevelEdges.h"
#include "../Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../Surface/Helper/ActionModelSurfaceAddPolygon.h"
#include "../Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"
#include "../../../../lib/base/map.h"
#include "../../../../Mode/Model/Mesh/ModeModelMesh.h"
#include <assert.h>

ActionModelBevelEdges::ActionModelBevelEdges(float _length)
{
	length = _length;
}

void *ActionModelBevelEdges::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(ModelSurface &s, m->Surface, i){
		BevelSurface(m, &s, i);
		_foreach_it_.update();
	}
	return NULL;
}

struct VertexData
{
	int v;
	//Map<int, int> edge_no;
	Array<int> edge, edge_dir;
	Array<int> poly, poly_side;
	bool closed;
};

struct VertexToCome
{
	vector pos;
	int bone;
	int ref_count;
	int v;
	VertexToCome(){	ref_count = 0;	}
};

struct PolygonToCome
{
	Array<VertexToCome*> v;
	void add(VertexToCome *vv)
	{
		for (int k=0; k<v.num; k++)
			if (vv == v[k])
				return;
		v.add(vv);
		vv->ref_count ++;
	}
};

struct PolygonRelink
{
	Array<VertexToCome*> v;
	Array<vector> sv;
	void init(ModelPolygon *p)
	{
		if (v.num == 0){
			v.resize(p->Side.num * 2);
			foreach(VertexToCome *&vv, v)
				vv = NULL;
			sv.resize(p->Side.num * 2 * MATERIAL_MAX_TEXTURES);
		}
	}
	void relink(ModelPolygon *p, int side, VertexToCome *vv)
	{
		init(p);
		v[side*2] = vv;
		vv->ref_count ++;
	}
	void relink(ModelPolygon *p, int side, VertexToCome *a, VertexToCome *b)
	{
		init(p);
		v[side*2  ] = a;
		v[side*2+1] = b;
		a->ref_count ++;
		b->ref_count ++;
	}
};

int get_next_edge(ModelSurface *s, int edge, int ek, int dir, int &next_dir)
{
	ModelEdge &e = s->Edge[edge];
	if (e.RefCount < 2)
		return -1;
	int np = (ek + dir) % 2;
	ModelPolygon &p = s->Polygon[e.Polygon[np]];
	int side = (e.Side[np] + p.Side.num + dir) % p.Side.num;
	next_dir = p.Side[side].EdgeDirection;
	return p.Side[side].Edge;
}

void ActionModelBevelEdges::build_vertices(Array<VertexToCome> &vv, DataModel *m)
{
	foreach(VertexToCome &v, vv)
		if (v.ref_count > 0){
			v.v = m->Vertex.num;
			AddSubAction(new ActionModelAddVertex(v.pos, v.bone), m);
		}
}

void ActionModelBevelEdges::do_poly_relink(ModelPolygon &p, PolygonRelink &r, int i, int surface, DataModel *m)
{
	Array<int> v;
	int material = p.Material;
	Array<vector> sv;

	SkinGeneratorMulti sg;
	sg.init_polygon(m->Vertex, p);

	//msg_write("r!");
	for (int k=0;k<p.Side.num;k++){
		if ((r.v[k*2]) && (r.v[k*2+1])){
			//msg_write(format("%d: 2x", k));
			v.add(r.v[k*2]->v);
			for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
				sv.add(sg.get(r.v[k*2]->pos, l));
			v.add(r.v[k*2+1]->v);
			for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
				sv.add(sg.get(r.v[k*2+1]->pos, l));
		}else if (r.v[k*2]){
			//msg_write(format("%d: 1x", k));
			v.add(r.v[k*2]->v);
			for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
				sv.add(sg.get(r.v[k*2]->pos, l));
		}else{
			v.add(p.Side[k].Vertex);
			for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
				sv.add(p.Side[k].SkinVertex[l]);
		}
	}
	//msg_write(ia2s(v));
	//sv.resize(v.num);

	// transpose sv
	Array<vector> ssv;
	ssv.resize(v.num * MATERIAL_MAX_TEXTURES);
	for (int k=0; k<v.num; k++)
		for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
			ssv[k + l*v.num] = sv[k*MATERIAL_MAX_TEXTURES + l];

	// relink
	AddSubAction(new ActionModelSurfaceDeletePolygon(surface, i), m);
	AddSubAction(new ActionModelSurfaceAddPolygon(surface, v, material, ssv, i), m);
}


static Array<VertexToCome> ev[2];
static Array<Array<VertexToCome> > pv;

void add_edge_neighbour(ModelSurface *s, ModelEdge &e, int k, int dir, PolygonToCome &pp)
{
	ModelPolygon &p = s->Polygon[e.Polygon[k]];
	int nei_side = (e.Side[k] + p.Side.num + 2*dir - 1) % p.Side.num;
	int nei = p.Side[nei_side].Edge;
	if (s->Edge[nei].is_selected){
		pp.add(&pv[e.Polygon[k]][(e.Side[k] + dir) % p.Side.num]);
	}else{
		pp.add(&ev[(p.Side[nei_side].EdgeDirection + dir + 1) % 2][nei]);
	}
}

void ActionModelBevelEdges::BevelSurface(DataModel *m, ModelSurface *s, int surface)
{
	ev[0].resize(s->Edge.num);
	ev[1].resize(s->Edge.num);
	pv.resize(s->Polygon.num);

	Array<VertexData> vdata;
	vdata.resize(s->Vertex.num);
	Array<int> obsolete_vertex;

	// (potentially) new vertices on edges
	foreachi(ModelEdge &e, s->Edge, i){
		vector d = m->Vertex[e.Vertex[1]].pos - m->Vertex[e.Vertex[0]].pos;
		d.normalize();
		ev[0][i].pos = m->Vertex[e.Vertex[0]].pos + d * length;
		ev[0][i].bone = m->Vertex[e.Vertex[0]].BoneIndex;
		ev[1][i].pos = m->Vertex[e.Vertex[1]].pos - d * length;
		ev[1][i].bone = m->Vertex[e.Vertex[1]].BoneIndex;
	}

	// (potentially) new vertices in polygons
	foreachi(ModelPolygon &p, s->Polygon, i){
		pv[i].resize(p.Side.num);
		for (int k=0;k<p.Side.num;k++){
			vector dir0 = m->Vertex[p.Side[(k+p.Side.num-1)%p.Side.num].Vertex].pos - m->Vertex[p.Side[k].Vertex].pos;
			vector dir1 = m->Vertex[p.Side[(k+1           )%p.Side.num].Vertex].pos - m->Vertex[p.Side[k].Vertex].pos;
			dir0.normalize();
			dir1.normalize();
			pv[i][k].pos = m->Vertex[p.Side[k].Vertex].pos + (dir0 + dir1) * length;
			pv[i][k].bone = m->Vertex[p.Side[k].Vertex].BoneIndex;
		}
	}

	Array<PolygonToCome> new_poly;

	// closedness at vertices
	foreachi(int v, s->Vertex, vi)
		if (m->Vertex[v].is_selected){
			VertexData vd;
			vd.v = v;
			vd.closed = true;

			foreach(ModelEdge &e, s->Edge)
				if ((e.Vertex[0] == v) || (e.Vertex[1] == v))
					vd.closed &= (e.RefCount == 2);
			vdata[vi] = vd;
			obsolete_vertex.add(v);
		}

	// edges...
	foreachi(ModelEdge &e, s->Edge, ei)
		if (e.is_selected){
			// selected -> new polygon
			if (e.RefCount < 2)
				continue;
			PolygonToCome pp;

			add_edge_neighbour(s, e, 0, 0, pp);
			add_edge_neighbour(s, e, 1, 1, pp);
			add_edge_neighbour(s, e, 1, 0, pp);
			add_edge_neighbour(s, e, 0, 1, pp);

			new_poly.add(pp);
		}else{
			for (int k=0; k<2; k++)
				if (m->Vertex[e.Vertex[k]].is_selected){
					// not selected but vertex selected -> cut
					ev[k][ei].ref_count ++;
				}
		}

	// close vertices
	foreachi(int v, s->Vertex, vi)
		if ((m->Vertex[v].is_selected) && (vdata[vi].closed)){
			PolygonToCome pp;
			int edge = -1, edgedir;
			// find first edge
			foreachi(ModelEdge &e, s->Edge, ei){
				for (int k=0; k<2; k++)
					if (e.Vertex[k] == v){
						edge = ei;
						edgedir = k;
					}
				if (edge >= 0)
					break;
			}
			if (edge < 0)
				throw ActionException("BevelPoly: no edge at closed vertex found!");
			int edge0 = edge;
			do{
				if (ev[edgedir][edge].ref_count > 0)
					pp.add(&ev[edgedir][edge]);
				else{
					ModelEdge &e = s->Edge[edge];
					VertexToCome &v = pv[e.Polygon[  edgedir]][ e.Side[  edgedir]];
					if (v.ref_count > 0)
						pp.add(&v);
				}

				edge = get_next_edge(s, edge, edgedir, 1, edgedir);
				if (edge < 0)
					throw ActionException("BevelPoly: no next edge at closed vertex found!");
			}while(edge != edge0);
			pp.v.reverse();

			if (pp.v.num > 2)
				new_poly.add(pp);
		}

	// relink polys
	Array<PolygonRelink> pr;
	pr.resize(s->Polygon.num);
	foreachi(ModelPolygon &p, s->Polygon, i){
		bool relink = false;
		for (int k=0; k<p.Side.num; k++)
			relink |= m->Vertex[p.Side[k].Vertex].is_selected;
		if (!relink)
			continue;
		PolygonRelink r;
		for (int k=0; k<p.Side.num; k++){
			int kk = (k+p.Side.num-1) % p.Side.num;
			if (pv[i][k].ref_count > 0){
				r.relink(&p, k, &pv[i][k]);
			}else{
				VertexToCome *v_in  = &ev[1-p.Side[kk].EdgeDirection][p.Side[kk].Edge];
				VertexToCome *v_out = &ev[  p.Side[k ].EdgeDirection][p.Side[k ].Edge];
				bool r_in  = (v_in->ref_count > 0);
				bool r_out = (v_out->ref_count > 0);
				if (r_in && r_out)
					r.relink(&p, k, v_in, v_out);
				else if (r_in)
					r.relink(&p, k, v_in);
				else if (r_out)
					r.relink(&p, k, v_out);
			}
		}

		pr[i] = r;
	}

	// really build stuff
	build_vertices(ev[0], m);
	build_vertices(ev[1], m);
	foreach(Array<VertexToCome> &vv, pv)
		build_vertices(vv, m);

	// relink
	foreachi(PolygonRelink &r, pr, i)
		if (r.v.num > 0)
			do_poly_relink(s->Polygon[i], r, i, surface, m);

	// new polygons
	foreach(PolygonToCome &p, new_poly){
		Array<int> v;
		for (int k=0;k<p.v.num;k++)
			v.add(p.v[k]->v);
		//msg_write(ia2s(v));
		AddSubAction(new ActionModelAddPolygonAutoSkin(v, mode_model_mesh->CurrentMaterial), m);
	}

	// remove obsolete vertices
	foreachb(int v, obsolete_vertex)
		AddSubAction(new ActionModelDeleteUnusedVertex(v), m);

	ev[0].clear();
	ev[1].clear();
	pv.clear();
}

