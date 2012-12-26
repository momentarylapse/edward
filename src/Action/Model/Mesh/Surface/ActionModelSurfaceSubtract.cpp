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

float ActionModelSurfaceSubtract::sCol::get_f(DataModel *m, ModelPolygon *t)
{
	if (type == TYPE_OLD_VERTEX)
		return 0;
	if (type == TYPE_OWN_EDGE_OUT)
		return p.factor_between(m->Vertex[t->Side[side].Vertex].pos, m->Vertex[t->Side[(side + 1) % t->Side.num].Vertex].pos);
	throw ActionException("subtract: unhandled col type");
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

	// collide polygon <-> surface's edges
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
			t_col.add(sCol(col, sCol::TYPE_OTHER_EDGE, t_index, ei, -1));
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

			vector col;
			for (int i=0;i<vv2.num;i+=3){
				if (!LineIntersectsTriangle2(pl2, v2[vv2[i+0]], v2[vv2[i+1]], v2[vv2[i+2]], ve[0], ve[1], col, false))
					continue;
				int type = ((pl2.distance(ve[0]) > 0) ^ (t->Side[kk].EdgeDirection == 1)) ? sCol::TYPE_OWN_EDGE_IN : sCol::TYPE_OWN_EDGE_OUT;
				t_col.add(sCol(col, type, ti, t->Side[kk].Edge, kk));
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
#endif

void ActionModelSurfaceSubtract::sort_t_col(ModelSurface *s, Array<sCol> &c2)
{
	msg_db_r("sort_t_col", 2);
	msg_write(format("sort %d   %d", t_col.num, c2.num));
	foreach(sCol &cc, t_col)
		msg_write(format("%d  %d  - %d", cc.type, cc.polygon, cc.edge));

	// find first
	int last_poly = -1;
	foreachi(sCol &c, t_col, i)
		if (c.type == c.TYPE_OWN_EDGE_IN){
			last_poly = c.polygon;
			msg_write(c.polygon);
			c2.add(c);
			t_col.erase(i);
			break;
		}
	if (c2.num != 1){
		// TODO: completely inside....
		msg_db_l(2);
		throw ActionException("subtract: sort inconsistent: no start found");
	}

	while(true){
		msg_write(".");
		// find col on same s.poly as the last col
		bool found = false;
		foreachi(sCol &c, t_col, i)
			if (c.type == c.TYPE_OTHER_EDGE){
				for (int k=0;k<2;k++)
					if (s->Edge[c.edge].Polygon[k] == last_poly){
						msg_write(format("%d  %d  - %d", c.type, c.polygon, c.edge));
						c2.add(c);
						last_poly = s->Edge[c.edge].Polygon[1 - k];
						t_col.erase(i);
						found = true;
						break;
					}
				if (found)
					break;
			}

		if (found)
			continue;


		// find col on same s.edge as the last col
		foreachi(sCol &c, t_col, i)
			if ((c.type == c.TYPE_OWN_EDGE_OUT) && (c.polygon == last_poly)){
				msg_write(format("%d  %d  - %d", c.type, c.polygon, c.edge));
				c2.add(c);
				t_col.erase(i);
				found = true;
				break;
			}
		if (!found){
			msg_db_l(2);
			throw ActionException("subtract: sort inconsistent: no end found");
		}
		break;
	}
	msg_write("----------");

	int n = 0;
	foreach(sCol &c, t_col)
		if (c.type == c.TYPE_OTHER_EDGE)
			n ++;
	msg_write(format("%d  %d", n, t_col.num));

	// remove double vertices ????
	/*for (int i=c2.num-2;i>=2;i-=2)
		c2.erase(i);*/

	msg_db_l(2);
}

void ActionModelSurfaceSubtract::sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c, bool inverse)
{
	msg_db_r("sort_and_join_contours", 1);
	Array<sCol> v;
//	msg_write("---------------------------------");
	for (int k=0;k<t->Side.num;k++){
		vector pos = m->Vertex[t->Side[k].Vertex].pos;
		if (!b->IsInside(pos))
			v.add(sCol(pos, k));
	}

	if (inverse)
		for (int l=0;l<c.num;l++)
			c[l].reverse();

	Array<sCol> cc;
	cc = c[0];
	c.erase(0);

	while((c.num > 0) || (v.num > 0)){
		int side = cc.back().side;
		float f = cc.back().get_f(m, t);

		// search new contours
		float fmin = 2;
		int imin = -1;
		foreachi(Array<sCol> &ccc, c, i)
			if (ccc[0].side == side){
				float ff = cc.back().get_f(m, t);
				if ((ff > f) && (ff < fmin)){
					fmin = ff;
					imin = i;
				}
			}

		// add new contour
		if (imin >= 0){
			cc.append(c[imin]);
			c.erase(imin);
			continue;
		}

		// search old vertices
		bool ok = false;
		foreachi(sCol &ccc, v, i){
			if (ccc.side == ((side + 1) % t->Side.num)){
				cc.add(ccc);
				v.erase(i);
				ok = true;
				break;
			}
		}

		// nothing?
		if (!ok)
			throw ActionException("no next point found...");

	}

	foreachi(sCol &ccc, cc, i)
		ed->multi_view_3d->AddMessage3d(i2s(i + 1), ccc.p);

/*	// any triangle vertices to keep?
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
	}*/
	msg_db_l(1);
}

void ActionModelSurfaceSubtract::PolygonSubtract(DataModel *m, ModelSurface *&a, ModelPolygon *t, ModelSurface *&b, bool inverse)
{
#if 0
	msg_db_r("PolygonSubtract", 1);
	a->TestSanity("tria sub a prae");
	b->TestSanity("tria sub b prae");
	int a_i = m->get_surf_no(a);
	int b_i = m->get_surf_no(b);
#endif

	Array<Array<sCol> > c;

	// connect "cutting" contours
	while(t_col.num > 0){
//		msg_write(t_col.num);
		Array<sCol> cc;

		// find consecutive vertices
		sort_t_col(b, cc);

		c.add(cc);
	}

//	msg_write(format("contours: %d", c.num));

	sort_and_join_contours(m, t, b, c, inverse);

#if 0
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
