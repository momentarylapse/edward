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
#include "../../../../Data/Model/ModelMesh.h"
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

	bevelSurface(m->edit_mesh);
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
	ivec4 bone;
	vec4 bone_weight;
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
			v.resize(p->side.num * 2);
			for (VertexToCome *&vv: v)
				vv = NULL;
			sv.resize(p->side.num * 2 * MATERIAL_MAX_TEXTURES);
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

int get_next_edge(ModelMesh *m, int edge, int ek, int dir, int &next_dir)
{
	ModelEdge &e = m->edge[edge];
	if (e.ref_count < 2)
		return -1;
	int np = (ek + dir) % 2;
	ModelPolygon &p = m->polygon[e.polygon[np]];
	int side = (e.side[np] + p.side.num + dir) % p.side.num;
	next_dir = p.side[side].edge_direction;
	return p.side[side].edge;
}

void ActionModelBevelEdges::build_vertices(Array<VertexToCome> &vv, DataModel *m)
{
	for (VertexToCome &v: vv)
		if (v.ref_count > 0){
			v.v = m->edit_mesh->vertex.num;
			addSubAction(new ActionModelAddVertex(v.pos, v.bone, v.bone_weight), m);
		}
}

void ActionModelBevelEdges::do_poly_relink(ModelPolygon &p, PolygonRelink &r, int i, DataModel *m)
{
	Array<int> v;
	int material = p.material;
	Array<vector> sv;

	SkinGeneratorMulti sg;
	sg.init_polygon(m->edit_mesh->vertex, p);

	//msg_write("r!");
	for (int k=0;k<p.side.num;k++){
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
			v.add(p.side[k].vertex);
			for (int l=0; l<MATERIAL_MAX_TEXTURES; l++)
				sv.add(p.side[k].skin_vertex[l]);
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
	addSubAction(new ActionModelSurfaceDeletePolygon(i), m);
	addSubAction(new ActionModelSurfaceAddPolygon(v, material, ssv, i), m);
}


static Array<VertexToCome> ev[2];
static Array<Array<VertexToCome> > pv;

void add_edge_neighbour(ModelMesh *s, ModelEdge &e, int k, int dir, PolygonToCome &pp) {
	ModelPolygon &p = s->polygon[e.polygon[k]];
	int nei_side = (e.side[k] + p.side.num + 2*dir - 1) % p.side.num;
	int nei = p.side[nei_side].edge;
	if (s->edge[nei].is_selected){
		pp.add(&pv[e.polygon[k]][(e.side[k] + dir) % p.side.num]);
	}else{
		pp.add(&ev[(p.side[nei_side].edge_direction + dir + 1) % 2][nei]);
	}
}

void ActionModelBevelEdges::bevelSurface(ModelMesh *m)
{
	// seems a bit wasteful...
	ev[0].resize(m->edge.num);
	ev[1].resize(m->edge.num);
	pv.resize(m->polygon.num);

	Array<VertexData> vdata;
	vdata.resize(m->vertex.num);
	Array<int> obsolete_vertex;

	// (potentially) new vertices on edges
	foreachi(ModelEdge &e, m->edge, i){
		vector d = m->vertex[e.vertex[1]].pos - m->vertex[e.vertex[0]].pos;
		d.normalize();
		ev[0][i].pos = m->vertex[e.vertex[0]].pos + d * length;
		ev[0][i].bone = m->vertex[e.vertex[0]].bone_index;
		ev[0][i].bone_weight = m->vertex[e.vertex[0]].bone_weight;
		ev[1][i].pos = m->vertex[e.vertex[1]].pos - d * length;
		ev[1][i].bone = m->vertex[e.vertex[1]].bone_index;
		ev[1][i].bone_weight = m->vertex[e.vertex[1]].bone_weight;
	}

	// (potentially) new vertices in polygons
	foreachi(ModelPolygon &p, m->polygon, i){
		pv[i].resize(p.side.num);
		for (int k=0;k<p.side.num;k++){
			vector dir0 = m->vertex[p.side[(k+p.side.num-1)%p.side.num].vertex].pos - m->vertex[p.side[k].vertex].pos;
			vector dir1 = m->vertex[p.side[(k+1           )%p.side.num].vertex].pos - m->vertex[p.side[k].vertex].pos;
			dir0.normalize();
			dir1.normalize();
			pv[i][k].pos = m->vertex[p.side[k].vertex].pos + (dir0 + dir1) * length;
			pv[i][k].bone = m->vertex[p.side[k].vertex].bone_index;
			pv[i][k].bone_weight = m->vertex[p.side[k].vertex].bone_weight;
		}
	}

	Array<PolygonToCome> new_poly;

	// closedness at vertices
	foreachi(auto &v, m->vertex, vi)
		if (v.is_selected){
			VertexData vd;
			vd.v = vi;
			vd.closed = true;

			for (ModelEdge &e: m->edge)
				if ((e.vertex[0] == vi) || (e.vertex[1] == vi))
					vd.closed &= (e.ref_count == 2);
			vdata[vi] = vd;
			obsolete_vertex.add(vi);
		}

	// edges...
	foreachi(ModelEdge &e, m->edge, ei)
		if (e.is_selected){
			// selected -> new polygon
			if (e.ref_count < 2)
				continue;
			PolygonToCome pp;

			add_edge_neighbour(m, e, 0, 0, pp);
			add_edge_neighbour(m, e, 1, 1, pp);
			add_edge_neighbour(m, e, 1, 0, pp);
			add_edge_neighbour(m, e, 0, 1, pp);

			new_poly.add(pp);
		}else{
			for (int k=0; k<2; k++)
				if (m->vertex[e.vertex[k]].is_selected){
					// not selected but vertex selected -> cut
					ev[k][ei].ref_count ++;
				}
		}

	// close vertices
	foreachi(auto &v, m->vertex, vi)
		if ((v.is_selected) && (vdata[vi].closed)){
			PolygonToCome pp;
			int edge = -1, edgedir;
			// find first edge
			foreachi(ModelEdge &e, m->edge, ei){
				for (int k=0; k<2; k++)
					if (e.vertex[k] == vi){
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
					ModelEdge &e = m->edge[edge];
					VertexToCome &v = pv[e.polygon[  edgedir]][ e.side[  edgedir]];
					if (v.ref_count > 0)
						pp.add(&v);
				}

				edge = get_next_edge(m, edge, edgedir, 1, edgedir);
				if (edge < 0)
					throw ActionException("BevelPoly: no next edge at closed vertex found!");
			}while(edge != edge0);
			pp.v.reverse();

			if (pp.v.num > 2)
				new_poly.add(pp);
		}

	// relink polys
	Array<PolygonRelink> pr;
	pr.resize(m->polygon.num);
	foreachi(ModelPolygon &p, m->polygon, i){
		bool relink = false;
		for (int k=0; k<p.side.num; k++)
			relink |= m->vertex[p.side[k].vertex].is_selected;
		if (!relink)
			continue;
		PolygonRelink r;
		for (int k=0; k<p.side.num; k++){
			int kk = (k+p.side.num-1) % p.side.num;
			if (pv[i][k].ref_count > 0){
				r.relink(&p, k, &pv[i][k]);
			}else{
				VertexToCome *v_in  = &ev[1-p.side[kk].edge_direction][p.side[kk].edge];
				VertexToCome *v_out = &ev[  p.side[k ].edge_direction][p.side[k ].edge];
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
	build_vertices(ev[0], m->model);
	build_vertices(ev[1], m->model);
	for (Array<VertexToCome> &vv: pv)
		build_vertices(vv, m->model);

	// relink
	foreachi(PolygonRelink &r, pr, i)
		if (r.v.num > 0)
			do_poly_relink(m->polygon[i], r, i, m->model);

	// new polygons
	for (PolygonToCome &p: new_poly){
		Array<int> v;
		for (int k=0;k<p.v.num;k++)
			v.add(p.v[k]->v);
		//msg_write(ia2s(v));
		addSubAction(new ActionModelAddPolygonAutoSkin(v, mode_model_mesh->current_material), m->model);
	}

	// remove obsolete vertices
	foreachb(int v, obsolete_vertex)
		addSubAction(new ActionModelDeleteUnusedVertex(v), m->model);

	ev[0].clear();
	ev[1].clear();
	pv.clear();
}

