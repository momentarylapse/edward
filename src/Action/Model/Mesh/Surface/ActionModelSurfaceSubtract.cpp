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
#include "../../../../Edward.h"


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
	foreachb(ModelSurface &b, m->Surface){
		ModelSurface *bb = &b;
		if (bb->is_selected){
			foreachb(ModelSurface &a, m->Surface){
				ModelSurface *aa = &a;
				if ((aa->view_stage >= m->ViewStage) && (!aa->is_selected))
					SurfaceSubtract(m, aa, bb);
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
	t_col.clear();

	// polygon's data
	Array<vector> v;
	for (int k=0;k<t->Side.num;k++)
		v.add(m->Vertex[t->Side[k].Vertex].pos);
	plane pl;
	PlaneFromPointNormal(pl, m->Vertex[t->Side[0].Vertex].pos, t->TempNormal);

	Array<int> vv = t->Triangulate(m);

	// collide polygon <-> edges
	foreachi(ModelEdge &e, s->Edge, ei){
		vector ve[2];
		for (int k=0;k<2;k++)
			ve[k] = m->Vertex[e.Vertex[k]].pos;

		// crossing plane?
		if (pl.distance(ve[0]) * pl.distance(ve[1]) > 0)
			continue;

		vector col;
		for (int i=0;i<vv.num;i+=3){
			if (!LineIntersectsTriangle2(pl, v[vv[i+0]], v[vv[i+1]], v[vv[i+2]], ve[0], ve[1], col, false))
				continue;
			t_col.add(sCol(col, false, t_index, ei));
		}
	}

	// collide edges <-> polygons
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
			if (pl2.distance(ve[0]) * pl.distance(ve[1]) > 0)
				continue;

			vector col;
			for (int i=0;i<vv2.num;i+=3){
				if (!LineIntersectsTriangle2(pl2, v2[vv2[i+0]], v2[vv2[i+1]], v2[vv2[i+2]], ve[0], ve[1], col, false))
					continue;
				t_col.add(sCol(col, true, ti, kk));
			}
		}
	}

	// FIXME debug
	foreach(sCol &c, t_col)
		AddSubAction(new ActionModelAddVertex(c.p), m);
	msg_db_l(0);
	return t_col.num > 0;
}

#if 0
// t must not collide with s...
bool ActionModelSurfaceSubtract::PolygonInsideSurface(DataModel *m, ModelPolygon *t, ModelSurface *s)
{
	msg_db_r("PolygonInsideSurface", 2);
	vector p = m->Vertex[t->Vertex[0]].pos;
	vector d = e_z;

	// how many times does a ray starting at p hit s?
	int n = 0;
	foreach(ModelPolygon &t2, s->Polygon){
		vector v[3];
		for (int k=0;k<3;k++)
			v[k] = m->Vertex[t2.Vertex[k]].pos;
		vector col;
		if (!LineIntersectsTriangle(v[0], v[1], v[2], p, p + d, col, false))
			continue;
		if (VecDotProduct(col - p, d) < 0)
			continue;

		n ++;
	}
	msg_db_l(2);
	return (n % 2) == 1;
}

bool ActionModelSurfaceSubtract::sort_t_col(ModelSurface *s, Array<sCol> &c2)
{
	msg_db_r("sort_t_col", 2);
//	msg_write(format("sort %d   %d", t_col.num, c2.num));
//	foreach(t_col, cc)
//		msg_write(format("%d  %d  %d - %d", (int)cc->own_edge, cc->index, cc->k, s->Triangle[cc->index].EdgeIndex[cc->k]));

	// find first
	foreachi(sCol &c, t_col, i)
		if (c.own_edge){
//			msg_write(c.index);
			c2.add(c);
			t_col.erase(i);
			_foreach_it_.update(); // TODO badness 10000!!!!!!!!!
			break;
		}
	if (c2.num != 1){
		msg_error("subtract: first not found");
		// TODO: completely inside....
		msg_db_l(2);
		return false;
	}

	while(true){
//		msg_write(".");
		// find col on same s.tria as the last col
		bool found = false;
		foreachi(sCol &c, t_col, i)
			if (c.index == c2.back().index){
//				msg_write(format("%d  %d  %d - %d", (int)c->own_edge, c->index, c->k, s->Triangle[c->index].Edge[c->k]));
				c2.add(c);
				t_col.erase(i);
				_foreach_it_.update(); // TODO
				found = true;
			}
		if (!found){
			msg_error("subtract: inconsistent (1)");
			msg_db_l(2);
			return false;
		}
		if (c2.back().own_edge)
			break;
		found = false;

		// find col on same s.edge as the last col
		foreachi(sCol &c, t_col, i)
			if (!c.own_edge)
			if (s->Polygon[c.index].Edge[c.k] == s->Polygon[c2.back().index].Edge[c2.back().k]){
//				msg_write(format("%d  %d  %d - %d", (int)c.own_edge, c.index, c.k, s->Triangle[c.index].Edge[c.k]));
				c2.add(c);
				t_col.erase(i);
				_foreach_it_.update(); // TODO
				found = true;
			}
		if (!found){
			msg_error("subtract: inconsistent (2)");
			msg_db_l(2);
			return false;
		}
	}
//	msg_write("----------");

	/*int n = 0;
	foreach2(t_col, c)
		if (c->own_edge)
			n ++;
	msg_write(format("%d  %d", n, t_col.num));*/
	msg_db_l(2);
	return true;
}

void ActionModelSurfaceSubtract::sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c, bool inverse)
{
	msg_db_r("sort_and_join_contours", 1);
	vector v[3];
//	msg_write("---------------------------------");
	for (int k=0;k<3;k++)
		v[k] = m->Vertex[t->Vertex[k]].pos;

	// any contours in wrong direction?
	for (int l=0;l<c.num;l++){
		/*int k0 = c[l][0].k;*/
		int kb = c[l].back().k;
//		float f0 = VecLength(c[l][0].p - v[k0]) / VecLength(v[(k0 + 1) % 3] - v[k0]);
//		float f1 = VecLength(c[l].back().p - v[kb]) / VecLength(v[(kb + 1) % 3] - v[kb]);
//		msg_write(format("len: %d     %d - %d     %f   %f", c[l].num, c[l][0].k, c[l].back().k, f0, f1));
		vector nb = b->Polygon[c[l].back().index].TempNormal;
//		msg_write(format("k0 = %d  kb = %d        n=%d", k0, kb, mc[l].num));

		// wrong direction?
		float sign = inverse ? -1 : 1;
		if (sign * VecDotProduct(nb, (v[(kb + 1) % 3] - v[kb])) < 0){
//			msg_write("reverse");
			c[l].reverse();
		}
	}

	// any triangle vertices to keep?
	for (int l=0;l<c.num;l++){
//		msg_write(format("- cont %d / %d    %d - %d", l, c.num, c[l][0].k, c[l].back().k));
		int kb = c[l].back().k;

		float f_min = (c[l].back().p - v[kb]).length() / (v[(kb + 1) % 3] - v[kb]).length();
//		msg_write(f2s(f_min, 3));

		for (int kk=kb;kk<kb + 4;kk++){
//			msg_write(format("edge %d", kk));
			bool done = false;
			bool next_cont = false;
			int k = kk % 3;
			int k2 = (k + 1) % 3;

			int l_next = 666;

			while (l_next >= 0){
//				msg_write(".");
				l_next = -1;
				float f_max = 1;

				// find next contour on edge k
				for (int ll=l;ll<c.num;ll++){

					if (c[ll][0].k != k)
						continue;

					float f = (c[ll][0].p - v[k]).length() / (v[k2] - v[k]).length();
					if ((f > f_min) && (f < f_max)){
						l_next = ll;
						f_max = f;
					}
				}

				if (l_next >= 0){
//					msg_write(format("next: %d  %f", l_next, f_max));

					if (l_next == l){
						// loop closed
//						msg_write("--");
						done = true;
						break;
					}else{
						// merge with next contour
//						msg_write(c[l_next].back().k);
						int k_next = c[l_next].back().k;
						f_min = (c[l_next].back().p - v[k_next]).length() / (v[(k_next + 1) % 3] - v[k_next]).length();
						c[l].append(c[l_next]);
						c.erase(l_next);

						// jump to ending edge of next contour
						kk += (k_next - kk + 6) % 3 - 1;
//						msg_write(f2s(f_min, 3));
						next_cont = true;
						break;
					}
				}
			}
			if (done)
				break;
			if (next_cont)
				continue;

			// add original triangle vertex to the contour
			c[l].add(sCol(v[k2], true, -1, -1));
//			msg_write("o");

			f_min = 0;
		}
	}
	msg_db_l(1);
}
#endif

void ActionModelSurfaceSubtract::PolygonSubtract(DataModel *m, ModelSurface *&a, ModelPolygon *t, ModelSurface *&b, bool inverse)
{
#if 0
	msg_db_r("PolygonSubtract", 1);
	a->TestSanity("tria sub a prae");
	b->TestSanity("tria sub b prae");
	int a_i = m->get_surf_no(a);
	int b_i = m->get_surf_no(b);

	Array<Array<sCol> > c;

	// connect "cutting" contours
	while(t_col.num > 0){
//		msg_write(t_col.num);
		Array<sCol> cc;

		// find consecutive vertices
		if (!sort_t_col(b, cc))
			break;

		// remove double vertices
		for (int i=cc.num-2;i>=2;i-=2)
			cc.erase(i);

		c.add(cc);
	}
//	msg_write(format("contours: %d", c.num));

	sort_and_join_contours(m, t, b, c, inverse);

	// create new surfaces
	for (int l=0;l<c.num;l++){
		// create contour vertices
		Array<int> vv;
		Array<vector> sv;
		for (int i=0;i<c[l].num;i++){
			vv.add(m->Vertex.num);
			AddSubAction(new ActionModelAddVertex(c[l][i].p), m);
			/*for (int l=0;l<MODEL_MAX_TEXTURES;l++)
				sv.add()*/
		}

		// fill contour with triangles
		AddSubAction(new ActionModelAddPolygon(vv, 0, sv), m);
		// TODO
		/*SetAutoTexturingTriangle(m, t);
		m->CreateTrianglesFlat(vv);
		ResetAutoTexturing();*/
	}

	a = &m->Surface[a_i];
	b = &m->Surface[b_i];
	a->TestSanity("tria sub a post");

	msg_db_l(1);
#endif
}

void ActionModelSurfaceSubtract::SurfaceSubtractUnary(DataModel *m, ModelSurface *& a, ModelSurface *& b, bool inverse)
{
	msg_db_r("SurfSubtractUnary", 0);
	a->TestSanity("surf sub a prae");
	b->TestSanity("surf sub b prae");
#if 0
	int nsurf = m->Surface.num;
	int ai = m->get_surf_no(a);
	int bi = m->get_surf_no(b);
#endif


	foreachib(ModelPolygon &t, a->Polygon, i)
		if (CollidePolygonSurface(m, &t, b, i)){
			a->TestSanity("tria sub (ext) a prae");
			PolygonSubtract(m, a, &t, b, inverse);
			a->TestSanity("tria sub (ext) a post");
			_foreach_it_.update(); // TODO
		}
#if 0
	// collide both surfaces and create additional polygons (as new surfaces...)
	Set<int> to_del;
	foreachib(ModelPolygon &t, a->Polygon, i)
		if (CollidePolygonSurface(m, &t, b)){
			a->TestSanity("tria sub (ext) a prae");
			PolygonSubtract(m, a, &t, b, inverse);
			a->TestSanity("tria sub (ext) a post");
			to_del.add(i);
		}else if (PolygonInsideSurface(m, &t, b) != inverse){
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
		AddSubAction(new ActionModelSurfaceAutoWeld(ai, m->Surface.num - 1, 0.00001f, true), m);
	}
	a = &m->Surface[ai];
	b = &m->Surface[bi];
	a->TestSanity("surf sub a post");
	b->TestSanity("surf sub b post");

	if (inverse)
		AddSubAction(new ActionModelSurfaceInvert(ai), m);
#endif

	msg_db_l(0);
}


void ActionModelSurfaceSubtract::SurfaceSubtract(DataModel *m, ModelSurface *&a, ModelSurface *&b)
{
	msg_db_r("SurfSubtract", 0);

	int ai = m->get_surf_no(a);
	int bi = m->get_surf_no(b);
	bool closed = a->IsClosed;
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

	if (closed){
		AddSubAction(new ActionModelSurfaceAutoWeld(ai, ci, 0.00001f, true), m);
		a = &m->Surface[ai];
		b = &m->Surface[bi];
	}

	msg_db_l(0);
}
