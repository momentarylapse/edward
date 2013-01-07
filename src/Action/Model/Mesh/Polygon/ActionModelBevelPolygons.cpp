/*
 * ActionModelBevelPolygons.cpp
 *
 *  Created on: 06.01.2013
 *      Author: michi
 */

#include "ActionModelBevelPolygons.h"
#include "../Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Edge/ActionModelSplitEdge.h"
#include "Helper/ActionModelPolygonRemoveVertex.h"
#include "ActionModelAddPolygonAutoSkin.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/base/map.h"
#include <assert.h>

ActionModelBevelPolygons::ActionModelBevelPolygons(float _length)
{
	length = _length;
}

void *ActionModelBevelPolygons::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(ModelSurface &s, m->Surface, i){
		BevelSurface(m, &s, i);
		break;
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
		v.add(vv);
		vv->ref_count ++;
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

void ActionModelBevelPolygons::build_vertices(Array<VertexToCome> &vv, DataModel *m)
{
	foreach(VertexToCome &v, vv)
		if (v.ref_count > 0){
			v.v = m->Vertex.num;
			AddSubAction(new ActionModelAddVertex(v.pos, v.bone), m);
		}
}


static Array<VertexToCome> ev[2];
static Array<Array<VertexToCome> > pv;

bool add_edge_neighbour(ModelSurface *s, ModelEdge &e, int k, int dir, PolygonToCome &pp)
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

void ActionModelBevelPolygons::BevelSurface(DataModel *m, ModelSurface *s, int surface)
{
	// update edge selection...
	foreach(ModelEdge &e, s->Edge)
		e.is_selected = m->Vertex[e.Vertex[0]].is_selected && m->Vertex[e.Vertex[1]].is_selected;

	ev[0].resize(s->Edge.num);
	ev[1].resize(s->Edge.num);
	pv.resize(s->Polygon.num);

	Array<VertexData> vdata;
	vdata.resize(s->Vertex.num);

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

			if (vd.closed){
				// new polygon...
			}
		}

	// edges...
	foreachi(ModelEdge &e, s->Edge, ei)
		if (e.is_selected){
			if (e.RefCount < 2)
				continue;
			msg_write("edge");
			PolygonToCome pp;
			// debug
			//pp.add(&ev[0][ei]);
			//pp.add(&ev[1][ei]);

			add_edge_neighbour(s, e, 0, 0, pp);
			add_edge_neighbour(s, e, 1, 1, pp);
			add_edge_neighbour(s, e, 1, 0, pp);
			add_edge_neighbour(s, e, 0, 1, pp);

			new_poly.add(pp);
		}

	build_vertices(ev[0], m);
	build_vertices(ev[1], m);
	foreach(Array<VertexToCome> &vv, pv)
		build_vertices(vv, m);

	msg_write("polys");
	foreach(PolygonToCome &p, new_poly){
		Array<int> v;
		for (int k=0;k<p.v.num;k++)
			v.add(p.v[k]->v);
		msg_write(ia2s(v));
		AddSubAction(new ActionModelAddPolygonAutoSkin(v), m);
	}

	/*foreach(ModelPolygon &p, s->Polygon)
		if (p.is_selected){
			Array<int> v;
			for (int k=0; k<p.Side.num; k++){
				vector dir0 = m->Vertex[p.Side[(k+p.Side.num-1)%p.Side.num].Vertex].pos - m->Vertex[p.Side[k].Vertex].pos;
				vector dir1 = m->Vertex[p.Side[(k+1           )%p.Side.num].Vertex].pos - m->Vertex[p.Side[k].Vertex].pos;
				dir0.normalize();
				dir1.normalize();
				vector pos = m->Vertex[p.Side[k].Vertex].pos + (dir0 + dir1) * length;
				AddSubAction(new ActionModelAddVertex(pos, m->Vertex[p.Side[k].Vertex].BoneIndex), m);
			}
		}else{

		}*/

	ev[0].clear();
	ev[1].clear();
	pv.clear();
}

