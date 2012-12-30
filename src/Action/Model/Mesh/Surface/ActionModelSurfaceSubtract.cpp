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
	return format("[%d]\tp=%d\te=%d\ts=%d\t%.1f\t%.1f\t%.1f", type, polygon, edge, side, p.x, p.y, p.z);
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

bool ActionModelSurfaceSubtract::sCol::operator==(const sCol &other) const
{
	return (type == other.type) && (p == other.p) && (edge == other.edge) && (side == other.side);
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
	int last_poly = -1;
	foreachi(sCol &c, c_in, i)
		if (c.type == c.TYPE_OWN_EDGE_IN){
			last_poly = c.polygon;
			c_out.add(c);
			c_in.erase(i);
			break;
		}

	if (last_poly < 0)
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
				/*if (inverse)
					c_out.reverse();*/
				return true;
			}

		msg_write(c_out.num);
		c_out += c_in;
		msg_error("evil contour");
		foreachi(sCol &c, c_out, i)
			msg_write(i2s(i) + " " + c.str());
		throw ActionException("subtract: contour starting on boundary but not ending on boundary found");
	}
	return false;
}

bool ActionModelSurfaceSubtract::find_contour_inside(DataModel *m, ModelPolygon *t, ModelSurface *s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse)
{
	if (c_in.num == 0)
		return false;

	if (c_in[0].type != sCol::TYPE_OTHER_EDGE)
		throw ActionException("internal contour without internal point..." + i2s(c_in[0].type));
	c_out.add(c_in[0]);
	c_in.erase(0);
	vector edge_dir = m->Vertex[s->Edge[c_out[0].edge].Vertex[1]].pos - m->Vertex[s->Edge[c_out[0].edge].Vertex[0]].pos;
	int last_poly = s->Edge[c_out[0].edge].Polygon[0];
	if (t->TempNormal * edge_dir < 0)
		last_poly = s->Edge[c_out[0].edge].Polygon[1];


	//throw ActionException("internal contour not implemented");

	while(true){
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

		// TODO ...test if loop is closed
		return true;
	}

	return false;
}

void ActionModelSurfaceSubtract::find_contours(DataModel *m, ModelPolygon *t, ModelSurface *s, Array<Array<sCol> > &c_out, bool inverse)
{
	int ni = 0, no = 0;
	foreach(sCol &cc, col){
		if (cc.type == cc.TYPE_OWN_EDGE_IN)
			ni ++;
		if (cc.type == cc.TYPE_OWN_EDGE_OUT)
			no ++;
	}
	msg_write(format("%d  %d", ni, no));
	if (ni != no)
		throw ActionException("ni != no");

	Array<sCol> temp;
	while (find_contour_boundary(s, col, temp, inverse)){
		c_out.add(temp);
		temp.clear();
	}

	while (find_contour_inside(m, t, s, col, temp, inverse)){
		c_out.add(temp);
		temp.clear();
	}

	if (inverse){
		foreach(Array<sCol> &cc, c_out)
			cc.reverse();
	}

	sort_and_join_contours(m, t, s, c_out, inverse);

	foreach(Array<sCol> &cc, c_out){
		msg_write("contour");
		foreachi(sCol &c, cc, i)
			msg_write(i2s(i) + " " + c.str());
		if (cc.num < 3)
			throw ActionException("contour with num<3");
	}
}

void ActionModelSurfaceSubtract::sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c_in, bool inverse)
{
	msg_db_r("sort_and_join_contours", 1);

	// find old vertices
	Array<sCol> v;
	for (int k=0;k<t->Side.num;k++){
		vector pos = m->Vertex[t->Side[k].Vertex].pos;
		if (b->IsInside(pos) == inverse)
			v.add(sCol(pos, k));
	}

	Array<Array<sCol> > c_out;

	// find purely internal contours
	bool boundary_points = false;
	for (int i=c_in.num-1; i>=0; i--)
		if (c_in[i][0].type == sCol::TYPE_OTHER_EDGE){
			c_out.add(c_in[i]);
			c_in.erase(i);
		}else{
			boundary_points = true;
		}

	// find contours on boundary and connect with old vertices
	while(c_in.num > 0){
		Array<sCol> cc = c_in.pop();
		//msg_write("------con");

		// expand current contour
		while(true){

			int side = cc.back().side;
			float f = cc.back().get_f(m, t);
			int side0 = cc[0].side;
			float f0 = cc[0].get_f(m, t);
			//msg_write(format("%d %f  -> %d %f", side0, f0, side, f));

			// loop already closed?
			bool closed = (side == side0) && (f0 > f);

			// search new contours
			float fmin = 2;
			if (closed) // don't cross the start/finish line
				fmin = f0;
			int imin = -1;
			foreachi(Array<sCol> &ccc, c_in, i)
				if (ccc[0].side == side){
					float ff = ccc[0].get_f(m, t);
					//msg_write(f2s(ff, 3));
					if ((ff > f) && (ff < fmin)){
						//msg_write("found");
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

			// don't add old vertices when closed
			if (closed)
				break;

			// search old vertices
			bool found = false;
			foreachi(sCol &ccc, v, i){
				if (ccc.side == ((side + 1) % t->Side.num)){
					cc.add(ccc);
					v.erase(i);
					found = true;
					break;
				}
			}
			if (!found)
				break;
		}

		// done?
		/*foreachi(sCol &ccc, cc, i)
			ed->multi_view_3d->AddMessage3d(i2s(i + 1), ccc.p);*/
		c_out.add(cc);
	}


	// without boundary contours the old vertices build their own contour
	if (v.num > 0){
		if (boundary_points)
			throw ActionException("unused old points with boundary contours...");
		c_out.add(v);
	}


	c_in = c_out;
	msg_db_l(1);
}


float ActionModelSurfaceSubtract::get_ang(Array<sCol> &c, int i, const vector &flat_n)
{
	int ia = i - 1;
	int ic = i + 1;
	if (ia < 0)
		ia = c.num -1;
	if (ic >= c.num)
		ic = 0;
	vector v1 = c[i].p - c[ia].p;
	vector v2 = c[ic].p - c[i].p;
	v1.normalize();
	v2.normalize();
	float x = (v1 ^ v2) * flat_n;
	float y = v1 * v2;
	return atan2(x, y);
}

bool ActionModelSurfaceSubtract::vertex_in_tria(sCol &a, sCol &b, sCol &c, sCol &v, float &slope)
{
	float f, g;
	GetBaryCentric(v.p, b.p, c.p, a.p, f, g);
	slope = f / g;
	return ((f > 0) && (g > 0) && (f + g < 1));
}

void ActionModelSurfaceSubtract::combine_contours(Array<Array<sCol> > &c, int ca, int ia, int cb, int ib)
{
	Array<sCol> temp = c[ca].sub(ia, c[ca].num - ia) + c[ca].sub(0, ia);
	c[ca] = temp;

	// copy one point
	temp = c[cb].sub(ib, c[cb].num - ib) + c[cb].sub(0, ib + 1);
	c[ca].append(temp);

	c.erase(cb);
}

void ActionModelSurfaceSubtract::triangulate_contours(DataModel *m, ModelPolygon *t, Array<Array<sCol> > &contours)
{
	if (contours.num == 1)
		return;
	Array<Array<sCol> > temp = contours;
	Array<Array<sCol> > output;

	while(contours.num > 0){

		// find largest angle (sharpest)
		// TODO: prevent colinear triangles!
		int i_max = -1;
		int c_max;
		float f_max = 0;
		int inside_i, inside_c;
		float inside_slope;
		foreachi(Array<sCol> &c, contours, ci)
		for (int i=0;i<c.num;i++){
			float f = get_ang(c, (i+1) % c.num, t->TempNormal);
			if (f < 0)
				continue;
			// cheat: ...
			float f_n = get_ang(c, (i+2) % c.num, t->TempNormal);
			float f_l = get_ang(c, i, t->TempNormal);
			if (f_n >= 0)
				f += 0.01f / (f_n + 0.01f);
			if (f_l >= 0)
				f += 0.01f / (f_l + 0.01f);

			if (f > f_max){
				inside_i = -1;
				inside_slope = -1;
				// other vertices within this triangle?
				bool ok = true;
				foreachi(Array<sCol> &other, contours, cj)
				for (int j=0;j<other.num;j++){
					if (&c == &other)
						if ((j == i) || (j == ((i+1) % c.num)) || (j == ((i+2) % c.num)))
							continue;
					float slope;
					if (vertex_in_tria(c[i], c[(i+1) % c.num], c[(i+2) % c.num], other[j], slope)){
						if (&c == &other){
							ok = false;
							break;
						}
						if ((slope < inside_slope) || (inside_i < 0)){
							inside_i = j;
							inside_c = cj;
							inside_slope = slope;
						}
					}
				}

				if (ok){
					f_max = f;
					i_max = i;
					c_max = ci;
				}
			}
		}

		if (i_max < 0){
			msg_error("could not fill contours");
			break;
			//throw ActionException("could not fill contours");
		}


		if (inside_i < 0){
			//msg_write("--");
			//msg_write(format("%d  %d", c_max, i_max));
			Array<sCol> tt;
			tt.add(contours[c_max][i_max]);
			tt.add(contours[c_max][(i_max+1) % contours[c_max].num]);
			tt.add(contours[c_max][(i_max+2) % contours[c_max].num]);
			output.add(tt);

			contours[c_max].erase((i_max+1) % contours[c_max].num);
			if (contours[c_max].num < 3)
				contours.erase(c_max);
		}else{
			//msg_write("-- inside");
			//msg_write(format("%d  %d   %d %d", c_max, i_max, inside_c, inside_i));
			Array<sCol> tt;
			tt.add(contours[c_max][i_max]);
			tt.add(contours[c_max][(i_max+1) % contours[c_max].num]);
			tt.add(contours[inside_c][inside_i]);
			output.add(tt);

			combine_contours(contours, c_max, (i_max+1) % contours[c_max].num, inside_c, inside_i);
		}
	}

	/*foreach(Array<sCol> &cc, output){
		msg_write("out");
		foreachi(sCol &c, cc, i)
			msg_write(i2s(i) + " " + c.str());
	}*/

	contours = output;
}

bool ActionModelSurfaceSubtract::combine_polygons(Array<Array<sCol> > &c, int ia, int ib)
{
	Array<int> equals;
	foreachi(sCol &a, c[ia], cia)
		foreachi(sCol &b, c[ib], cib)
			if (a == b){
				equals.add(cia);
				equals.add(cib);
			}
	if (equals.num != 4)
		return false;

	if ((equals[0] == 0) && (equals[2] == c[ia].num-1)){
		int t = equals[0];
		equals[0] = equals[2];
		equals[2] = t;
		t = equals[1];
		equals[1] = equals[3];
		equals[3] = t;
	}

	if ((equals[2] - equals[0] + c[ia].num - 1) % c[ia].num != 0)
		return false;
	if ((equals[3] - equals[1] + c[ib].num + 1) % c[ib].num != 0)
		return false;

	//msg_write("combine");
	//msg_write(ia2s(equals));
	Array<sCol> temp;
	if (equals[0] < equals[2])
		temp = c[ia].sub(0, equals[0] + 1);
	else
		temp = c[ia];
	if (equals[1] > equals[3]){
		temp += c[ib].sub(equals[1] + 1, -1);
		temp += c[ib].sub(0, equals[3]);
	}else{
		temp += c[ib].sub(1, c[ib].num - 2);
	}
	if (equals[0] < equals[2])
		temp += c[ia].sub(equals[2], -1);
	//msg_write(format("%d %d    %d", c[ia].num, c[ib].num, temp.num));
	c[ia] = temp;
	c.erase(ib);
	return true;
}

void ActionModelSurfaceSubtract::simplify_filling(Array<Array<sCol> > &c)
{
	if (c.num == 1)
		return;

	/*msg_write("prae sim");
	foreach(Array<sCol> &cc, c){
			msg_write("out");
			foreachi(sCol &ccc, cc, i)
				msg_write(i2s(i) + " " + ccc.str());
		}*/

	bool found = true;
	while(found){
		found = false;

		for (int ci=0;ci<c.num;ci++)
			for (int cj=ci+1;cj<c.num;cj++)
				if (combine_polygons(c, ci, cj)){
					ci = cj = c.num;
					found = true;
				}
	}


	/*msg_write("post sim");
	foreach(Array<sCol> &cc, c){
			msg_write("out");
			foreachi(sCol &ccc, cc, i)
				msg_write(i2s(i) + " " + ccc.str());
		}*/
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

	triangulate_contours(m, t, contours);

	simplify_filling(contours);

	SkinGeneratorMulti sg;
	sg.init_polygon(m, *t);

	// create new surfaces
	foreach(Array<sCol> &c, contours){
		if (inverse)
			c.reverse();

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
	bool closed = a->IsClosed;
	ModelSurface *c;
	int ci;

	if (closed){
		c = (ModelSurface*)AddSubAction(new ActionModelSurfaceCopy(bi), m);
		ci = m->get_surf_no(c);
		a = &m->Surface[ai];
		SurfaceSubtractUnary(m, c, a, true);
		b = &m->Surface[bi];
	}

	SurfaceSubtractUnary(m, a, b, false);
	//SurfaceSubtractUnary(m, a, b, true);

	if (closed)
		AddSubAction(new ActionModelSurfaceAutoWeld(ai, ci, 0.00001f), m);

	msg_db_l(0);
}
