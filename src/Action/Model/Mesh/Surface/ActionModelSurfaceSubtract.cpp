/*
 * ActionModelSurfaceSubtract.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceSubtract.h"
#include "ActionModelSurfaceAutoWeld.h"
#include "ActionModelSurfaceInvert.h"
#include "Helper/ActionModelSurfaceDeletePolygon.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygon.h"
#include "ActionModelSurfaceCopy.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"
#include "../../../../Edward.h"

float ActionModelSurfaceSubtract::sCol::get_f(DataModel *m, ModelPolygon *t)
{
	if (type == TYPE_OLD_VERTEX)
		return 0;
	if ((type == TYPE_OWN_EDGE_OUT) || (type == TYPE_OWN_EDGE_IN))
		return p.factor_between(m->Vertex[t->Side[side].Vertex].pos, m->Vertex[t->Side[(side + 1) % t->Side.num].Vertex].pos);
	throw ActionException("subtract: unhandled col type");
}

string ActionModelSurfaceSubtract::sCol::str() const
{
	return format("[%d]\tp=%d\te=%d\ts=%d", type, polygon, edge, side);
}

ActionModelSurfaceSubtract::sCol::sCol(const vector &_p, int _side)
{
	p = _p;
	type = TYPE_OLD_VERTEX;
	polygon = -1;
	edge = -1;
	side = _side;
}

ActionModelSurfaceSubtract::sCol::sCol(const vector &_p, int _type, int _polygon, int _edge, int _side)
{
	p = _p;
	type = _type;
	polygon = _polygon;
	edge = _edge;
	side = _side;
}


ActionModelSurfaceSubtract::ActionModelSurfaceSubtract()
{}

void *ActionModelSurfaceSubtract::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int n = 0;
	foreach(ModelSurface &s, m->Surface)
		if ((s.is_selected) && (s.IsClosed))
			n ++;
	if (n == 0){
		throw ActionException("no closed surfaces selected");
		//ed->SetMessage(_("Keine geschlossene Fl&achen markiert"));
		//return;
	}

	msg_db_r("Subtract", 1);
	for (int bi=m->Surface.num-1; bi>=0; bi--){
		if (m->Surface[bi].is_selected){
			for (int ai=m->Surface.num-1; ai>=0; ai--){
				ModelSurface *a = &m->Surface[ai];
				if ((a->view_stage >= m->ViewStage) && (!a->is_selected))
					SurfaceSubtract(m, a, &m->Surface[bi]);
			}
		}
	}
	ed->SetMessage(format(_("%d geschlossene Fl&achen subtrahiert"), n));
	msg_db_l(1);
	return NULL;
}

#if 0
bool ActionModelSurfaceSubtract::CollidePolygons(DataModel *m, ModelPolygon *t1, ModelPolygon *t2, int t2_index)
{
	msg_db_r("CollidePolygons", 1);
	Array<vector> v1, v2;
	for (int k=0;k<t1->Side.num;k++)
		v1.add(m->Vertex[t1->Side[k].Vertex].pos);
	for (int k=0;k<t2->Side.num;k++)
		v2.add(m->Vertex[t2->Side[k].Vertex].pos);
	plane pl1, pl2;
	PlaneFromPointNormal(pl1, m->Vertex[t1->Side[0].Vertex].pos, t1->TempNormal);
	PlaneFromPointNormal(pl2, m->Vertex[t2->Side[0].Vertex].pos, t2->TempNormal);
	bool bcol = false;


	// all vertices of t2 on the same side of pl1?
	bool all_same_side = true;
	for (int k=1;k<t2->Side.num;k++)
		if (pl1.distance(v2[0]) * pl1.distance(v2[k]) < 0)
			all_same_side = false;
	if (all_same_side){
		msg_db_l(1);
		return false;
	}

	// collide edges of t2 with pl1
	Array<int> vv1 = t1->Triangulate(m);
	for (int k=0;k<t2->Side.num;k++){
		int k2 = (k + 1) % t2->Side.num;
		vector col;
		for (int i=0;i<vv1.num;i+=3){
			if (!LineIntersectsTriangle2(pl1, v1[vv1[i+0]], v1[vv1[i+1]], v1[vv1[i+2]], v2[k], v2[k2], col, false))
				continue;
			if (!col.between(v2[k], v2[k2]))
				continue;
			t_col.add(sCol(col, false, t2_index, k));
			bcol = true;
		}
		//return true;
	}

	// collide edges of t1 with pl2
	Array<int> vv2 = t2->Triangulate(m);
	for (int k=0;k<t1->Side.num;k++){
		int k2 = (k + 1) % t1->Side.num;
		vector col;
		for (int i=0;i<vv2.num;i+=3){
			if (!LineIntersectsTriangle2(pl2, v2[vv2[i+0]], v2[vv2[i+1]], v2[vv2[i+2]], v1[k], v1[k2], col, false))
				continue;
			if (!col.between(v1[k], v1[k2]))
				continue;
			t_col.add(sCol(col, true, t2_index, k));
			bcol = true;
		}
		//return true;
	}


	foreach(sCol &c, t_col)
		AddSubAction(new ActionModelAddVertex(c.p), m);
	msg_db_l(1);
	return bcol;
}
#endif

bool ActionModelSurfaceSubtract::CollidePolygonSurface(DataModel *m, ModelPolygon *t, ModelSurface *s, int t_index)
{
	msg_db_r("CollidePolygonSurface", 0);
	col.clear();

	// polygon's data
	Array<vector> v;
	for (int k=0;k<t->Side.num;k++)
		v.add(m->Vertex[t->Side[k].Vertex].pos);
	plane pl;
	PlaneFromPointNormal(pl, m->Vertex[t->Side[0].Vertex].pos, t->TempNormal);

	Array<int> vv = t->Triangulate(m);

	// collide polygon <-> surface's edges
	foreachi(ModelEdge &e, s->Edge, ei){
		vector ve[2];
		for (int k=0;k<2;k++)
			ve[k] = m->Vertex[e.Vertex[k]].pos;

		// crossing plane?
		if (pl.distance(ve[0]) * pl.distance(ve[1]) > 0)
			continue;

		vector pos;
		for (int i=0;i<vv.num;i+=3){
			if (!LineIntersectsTriangle2(pl, v[vv[i+0]], v[vv[i+1]], v[vv[i+2]], ve[0], ve[1], pos, false))
				continue;
			col.add(sCol(pos, sCol::TYPE_OTHER_EDGE, t_index, ei, -1));
		}
	}

	// collide polygon's edges <-> surface's polygons
	foreachi(ModelPolygon &t2, s->Polygon, ti){
		// polygon's data
		Array<vector> v2;
		for (int k=0;k<t2.Side.num;k++)
			v2.add(m->Vertex[t2.Side[k].Vertex].pos);
		plane pl2;
		PlaneFromPointNormal(pl2, m->Vertex[t2.Side[0].Vertex].pos, t2.TempNormal);

		Array<int> vv2 = t2.Triangulate(m);
		for (int kk=0;kk<t->Side.num;kk++){
			vector ve[2];
			for (int k=0;k<2;k++)
				ve[k] = m->Vertex[t->Side[(kk + k) % t->Side.num].Vertex].pos;

			// crossing plane?
			if (pl2.distance(ve[0]) * pl2.distance(ve[1]) > 0)
				continue;

			vector pos;
			for (int i=0;i<vv2.num;i+=3){
				if (!LineIntersectsTriangle2(pl2, v2[vv2[i+0]], v2[vv2[i+1]], v2[vv2[i+2]], ve[0], ve[1], pos, false))
					continue;
				int type = (pl2.distance(ve[0]) > 0) ? sCol::TYPE_OWN_EDGE_IN : sCol::TYPE_OWN_EDGE_OUT;
				col.add(sCol(pos, type, ti, t->Side[kk].Edge, kk));
			}
		}
	}

	// FIXME debug
	/*foreach(sCol &c, col)
		AddSubAction(new ActionModelAddVertex(c.p), m);*/
	msg_db_l(0);
	return col.num > 0;
}

// we assume t does not collide with s...!
bool ActionModelSurfaceSubtract::PolygonInsideSurface(DataModel *m, ModelPolygon *t, ModelSurface *s)
{
	foreach(ModelPolygonSide &side, t->Side)
		if (!s->IsInside(m->Vertex[side.Vertex].pos))
			return false;
	return true;
}

bool ActionModelSurfaceSubtract::find_contour_boundary(ModelSurface *s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse)
{
	// find first
	int start = -1;
	int last_poly = -1;
	foreachi(sCol &c, c_in, i)
		if (c.type == c.TYPE_OWN_EDGE_IN){
			last_poly = c.polygon;
			//msg_write(c.polygon);
			start = c_out.num;
			c_out.add(c);
			c_in.erase(i);
			break;
		}

	if (start < 0)
		return false;


	while(true){
		//msg_write(".");
		// find col on same s.poly as the last col
		bool found = false;
		foreachi(sCol &c, c_in, i)
			if (c.type == c.TYPE_OTHER_EDGE){
				for (int k=0;k<2;k++)
					if (s->Edge[c.edge].Polygon[k] == last_poly){
						//msg_write(format("%d  %d  - %d", c.type, c.polygon, c.edge));
						c_out.add(c);
						last_poly = s->Edge[c.edge].Polygon[1 - k];
						c_in.erase(i);
						found = true;
						break;
					}
				if (found)
					break;
			}

		if (found)
			continue;


		// find col on same s.edge as the last col
		foreachi(sCol &c, c_in, i)
			if ((c.type == c.TYPE_OWN_EDGE_OUT) && (c.polygon == last_poly)){
				//msg_write(format("%d  %d  - %d", c.type, c.polygon, c.edge));
				c_out.add(c);
				c_in.erase(i);
				if (inverse)
					c_out.reverse();
				return true;
			}
		throw ActionException("subtract: contour starting on boundary but not ending on boundary found");
	}
	return false;
}

void ActionModelSurfaceSubtract::find_contours(DataModel *m, ModelPolygon *t, ModelSurface *s, Array<Array<sCol> > &c_out, bool inverse)
{
	Array<sCol> temp;
	while (find_contour_boundary(s, col, temp, inverse)){
		c_out.add(temp);
		temp.clear();
	}

	sort_and_join_contours(m, t, s, c_out, inverse);

	foreach(Array<sCol> &cc, c_out){
		msg_write("contour");
		foreachi(sCol &c, cc, i)
			msg_write(i2s(i) + " " + c.str());
	}
}

void ActionModelSurfaceSubtract::sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c_in, bool inverse)
{
	msg_db_r("sort_and_join_contours", 1);
	Array<sCol> v;
	for (int k=0;k<t->Side.num;k++){
		vector pos = m->Vertex[t->Side[k].Vertex].pos;
		if (!b->IsInside(pos))
			v.add(sCol(pos, k));
	}

	Array<Array<sCol> > c_out;
	Array<sCol> cc;
	cc = c_in[0];
	c_in.erase(0);

	while(true){
		int side = cc.back().side;
		float f = cc.back().get_f(m, t);
		int side0 = cc[0].side;
		float f0 = cc[0].get_f(m, t);

		// search new contours
		float fmin = 2;
		int imin = -1;
		foreachi(Array<sCol> &ccc, c_in, i)
			if (ccc[0].side == side){
				float ff = cc.back().get_f(m, t);
				if ((ff > f) && (ff < fmin)){
					fmin = ff;
					imin = i;
				}
			}

		// add new contour
		if (imin >= 0){
			cc.append(c_in[imin]);
			c_in.erase(imin);
			continue;
		}

		// search old vertices
		bool ok = false;
		foreachi(sCol &ccc, v, i){
			if (ccc.side == ((side + 1) % t->Side.num)){
				// loop already closed?
				if ((side == side0) && (f0 > f))
					break;
				cc.add(ccc);
				v.erase(i);
				ok = true;
				break;
			}
		}
		if (ok)
			continue;

		// done?
		/*foreachi(sCol &ccc, cc, i)
			ed->multi_view_3d->AddMessage3d(i2s(i + 1), ccc.p);*/
		c_out.add(cc);
		cc.clear();

		if (c_in.num > 0){
			cc = c_in[0];
			c_in.erase(0);
		}else if (v.num == 0){
			break;
		}else
			throw ActionException("no next point for contour found...");

	}
	c_in = c_out;
	msg_db_l(1);
}

void ActionModelSurfaceSubtract::PolygonSubtract(DataModel *m, ModelSurface *&a, ModelPolygon *t, int t_index, ModelSurface *&b, bool inverse)
{
	msg_db_r("PolygonSubtract", 0);
	a->TestSanity("tria sub a prae");
	b->TestSanity("tria sub b prae");
	int a_i = m->get_surf_no(a);
	int b_i = m->get_surf_no(b);

	// find contours
	Array<Array<sCol> > contours;
	find_contours(m, t, b, contours, inverse);

	SkinGeneratorMulti sg;
	sg.init_polygon(m, *t);

	// create new surfaces
	foreach(Array<sCol> &c, contours){
		// create contour vertices
		Array<int> vv;
		Array<vector> sv;
		for (int i=0;i<c.num;i++){
			vv.add(m->Vertex.num);
			AddSubAction(new ActionModelAddVertex(c[i].p), m);
		}

		// skin vertices
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			for (int i=0;i<c.num;i++)
				if (c[i].type == sCol::TYPE_OLD_VERTEX)
					sv.add(t->Side[c[i].side].SkinVertex[l]);
				else
					sv.add(sg.get(c[i].p, l));

		// fill contour with polygons
		AddSubAction(new ActionModelAddPolygon(vv, t->Material, sv), m);

		a = &m->Surface[a_i];
		b = &m->Surface[b_i];
		a->TestSanity("tria sub a post");

		t = &a->Polygon[t_index];
	}


	msg_db_l(0);
}

void ActionModelSurfaceSubtract::SurfaceSubtractUnary(DataModel *m, ModelSurface *& a, ModelSurface *& b, bool inverse)
{
	msg_db_r("SurfSubtractUnary", 0);
	a->TestSanity("surf sub a prae");
	b->TestSanity("surf sub b prae");
	int nsurf = m->Surface.num;
	int ai = m->get_surf_no(a);
	int bi = m->get_surf_no(b);

	// collide both surfaces and create additional polygons (as new surfaces...)
	Set<int> to_del;
	for (int i=a->Polygon.num-1; i>=0; i--)
		if (CollidePolygonSurface(m, &a->Polygon[i], b, i)){
			a->TestSanity("tria sub (ext) a prae");
			PolygonSubtract(m, a, &a->Polygon[i], i, b, inverse);
			a->TestSanity("tria sub (ext) a post");
			to_del.add(i);
		}else if (PolygonInsideSurface(m, &a->Polygon[i], b) != inverse){
			to_del.add(i);
		}
	a->TestSanity("surf sub a med");
	b->TestSanity("surf sub b med");

	// remove obsolete polygons
	foreachb(int p, to_del)
		AddSubAction(new ActionModelSurfaceDeletePolygon(ai, p), m);
	a->TestSanity("tria sub a med");

	// connect separate parts
	for (int i=m->Surface.num-1;i>=nsurf;i--){
		AddSubAction(new ActionModelSurfaceAutoWeld(ai, m->Surface.num - 1, 0.00001f), m);
	}
	a = &m->Surface[ai];
	b = &m->Surface[bi];
	a->TestSanity("surf sub a post");
	b->TestSanity("surf sub b post");

	if (inverse)
		AddSubAction(new ActionModelSurfaceInvert(ai), m);

	msg_db_l(0);
}


void ActionModelSurfaceSubtract::SurfaceSubtract(DataModel *m, ModelSurface *a, ModelSurface *b)
{
	msg_db_r("SurfSubtract", 0);

	int ai = m->get_surf_no(a);
	int bi = m->get_surf_no(b);
	bool closed = false;//a->IsClosed;
	ModelSurface *c;
	int ci;

	if (closed){
		c = (ModelSurface*)AddSubAction(new ActionModelSurfaceCopy(m, b), m);
		ci = m->get_surf_no(c);
		a = &m->Surface[ai];
		SurfaceSubtractUnary(m, c, a, true);
		b = &m->Surface[bi];
	}

	SurfaceSubtractUnary(m, a, b, false);

	if (closed)
		AddSubAction(new ActionModelSurfaceAutoWeld(ai, ci, 0.00001f), m);

	msg_db_l(0);
}
