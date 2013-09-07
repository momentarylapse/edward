/*
 * ModelGeometrySubtract.cpp
 *
 *  Created on: 23.08.2013
 *      Author: michi
 */

#include "ModelGeometry.h"
#include "../SkinGenerator.h"
#include "../../../Edward.h"



class sCol
{
public:
	enum{
		TYPE_OWN_EDGE_IN,
		TYPE_OWN_EDGE_OUT,
		TYPE_OTHER_EDGE,
		TYPE_OLD_VERTEX
	};
	sCol(){}
	sCol(const vector &_p, int _side);
	sCol(const vector &_p, int _type, int _polygon, int _edge, int _side);
	float get_f(ModelGeometry &m, ModelPolygon *t);
	bool operator==(const sCol &other) const;
	vector p;
	int type;
	int polygon, edge, side;
	string str() const;
};

Array<sCol> col;

bool CollidePolygons(ModelGeometry &m, ModelPolygon *t1, ModelPolygon *t2, int t2_index);
bool CollidePolygonSurface(ModelGeometry &a, ModelPolygon *pa, ModelGeometry &b, int t_index);
bool PolygonInsideSurface(ModelGeometry &m, ModelPolygon *t, ModelGeometry &s);
void find_contours(ModelGeometry &m, ModelPolygon *t, ModelGeometry &s, Array<Array<sCol> > &c_out, bool inverse);
bool find_contour_boundary(ModelGeometry &s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse);
bool find_contour_inside(ModelGeometry &m, ModelPolygon *t, ModelGeometry &s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse);
float get_ang(Array<sCol> &c, int i, const vector &flat_n);
bool vertex_in_tria(sCol &a, sCol &b, sCol &c, sCol &v, float &slope);
void combine_contours(Array<Array<sCol> > &c, int ca, int ia, int cb, int ib);
void triangulate_contours(ModelGeometry &m, ModelPolygon *t, Array<Array<sCol> > &c);
bool combine_polygons(Array<Array<sCol> > &c, int ia, int ib);
void simplify_filling(Array<Array<sCol> > &c);
void sort_and_join_contours(ModelGeometry &m, ModelPolygon *t, ModelGeometry &b, Array<Array<sCol> > &c, bool inverse);
void PolygonSubtract(ModelGeometry &a, ModelPolygon *t, int t_index, ModelGeometry &b, ModelGeometry &out, bool keep_inside);
bool SurfaceSubtractUnary(ModelGeometry &a, ModelGeometry &b, ModelGeometry &out, bool keep_inside);



float sCol::get_f(ModelGeometry &m, ModelPolygon *t)
{
	if (type == TYPE_OLD_VERTEX)
		return 0;
	if ((type == TYPE_OWN_EDGE_OUT) || (type == TYPE_OWN_EDGE_IN))
		return p.factor_between(m.Vertex[t->Side[side].Vertex].pos, m.Vertex[t->Side[(side + 1) % t->Side.num].Vertex].pos);
	throw ActionException("unhandled col type");
}

string sCol::str() const
{
	return format("[%d]\tp=%d\te=%d\ts=%d\t(%.1f\t%.1f\t%.1f)", type, polygon, edge, side, p.x, p.y, p.z);
}

sCol::sCol(const vector &_p, int _side)
{
	p = _p;
	type = TYPE_OLD_VERTEX;
	polygon = -1;
	edge = -1;
	side = _side;
}

sCol::sCol(const vector &_p, int _type, int _polygon, int _edge, int _side)
{
	p = _p;
	type = _type;
	polygon = _polygon;
	edge = _edge;
	side = _side;
}

bool sCol::operator==(const sCol &other) const
{
	return (type == other.type) && (p == other.p) && (edge == other.edge) && (side == other.side);
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

bool CollidePolygonSurface(ModelGeometry &a, ModelPolygon *pa, ModelGeometry &b, int t_index)
{
	msg_db_f("CollidePolygonSurface", 0);
	col.clear();

	// polygon's data
	Array<vector> v;
	for (int k=0;k<pa->Side.num;k++)
		v.add(a.Vertex[pa->Side[k].Vertex].pos);
	plane pl;
	PlaneFromPointNormal(pl, a.Vertex[pa->Side[0].Vertex].pos, pa->TempNormal);

	Array<int> vv = pa->Triangulate(a.Vertex);

	// collide polygon <-> surface's edges
	foreachi(ModelEdge &e, b.Edge, ei){
		vector ve[2];
		for (int k=0;k<2;k++)
			ve[k] = b.Vertex[e.Vertex[k]].pos;

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
	foreachi(ModelPolygon &pb, b.Polygon, ti){
		// polygon's data
		Array<vector> v2;
		for (int k=0;k<pb.Side.num;k++)
			v2.add(b.Vertex[pb.Side[k].Vertex].pos);
		plane pl2;
		PlaneFromPointNormal(pl2, b.Vertex[pb.Side[0].Vertex].pos, pb.TempNormal);

		Array<int> vv2 = pb.Triangulate(b.Vertex);
		for (int kk=0;kk<pa->Side.num;kk++){
			vector ve[2];
			for (int k=0;k<2;k++)
				ve[k] = a.Vertex[pa->Side[(kk + k) % pa->Side.num].Vertex].pos;

			// crossing plane?
			if (pl2.distance(ve[0]) * pl2.distance(ve[1]) > 0)
				continue;

			vector pos;
			for (int i=0;i<vv2.num;i+=3){
				if (!LineIntersectsTriangle2(pl2, v2[vv2[i+0]], v2[vv2[i+1]], v2[vv2[i+2]], ve[0], ve[1], pos, false))
					continue;
				int type = (pl2.distance(ve[0]) > 0) ? sCol::TYPE_OWN_EDGE_IN : sCol::TYPE_OWN_EDGE_OUT;
				col.add(sCol(pos, type, ti, pa->Side[kk].Edge, kk));
			}
		}
	}

	// FIXME debug
	/*foreach(sCol &c, col)
		AddSubAction(new ActionModelAddVertex(c.p), m);*/
	return col.num > 0;
}

// we assume t does not collide with s...!
bool PolygonInsideSurface(ModelGeometry &m, ModelPolygon *t, ModelGeometry &s)
{
	foreach(ModelPolygonSide &side, t->Side)
		if (!s.IsInside(m.Vertex[side.Vertex].pos))
			return false;
	return true;
}

bool find_contour_boundary(ModelGeometry &s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse)
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
					if (s.Edge[c.edge].Polygon[k] == last_poly){
						//msg_write(format("%d  %d  - %d", c.type, c.polygon, c.edge));
						c_out.add(c);
						last_poly = s.Edge[c.edge].Polygon[1 - k];
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
				return true;
			}

		msg_write(c_out.num);
		c_out += c_in;
		msg_error("evil contour");
		foreachi(sCol &c, c_out, i)
			msg_write(i2s(i) + " " + c.str());
		throw ActionException("contour starting on boundary but not ending on boundary found");
	}
	return false;
}

bool find_contour_inside(ModelGeometry &m, ModelPolygon *t, ModelGeometry &s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse)
{
	if (c_in.num == 0)
		return false;

	if (c_in[0].type != sCol::TYPE_OTHER_EDGE)
		throw ActionException("internal contour without internal point..." + i2s(c_in[0].type));
	c_out.add(c_in[0]);
	c_in.erase(0);
	vector edge_dir = s.Vertex[s.Edge[c_out[0].edge].Vertex[1]].pos - s.Vertex[s.Edge[c_out[0].edge].Vertex[0]].pos;
	int last_poly = s.Edge[c_out[0].edge].Polygon[0];
	if (t->TempNormal * edge_dir < 0)
		last_poly = s.Edge[c_out[0].edge].Polygon[1];


	//throw ActionException("internal contour not implemented");

	while(true){
		// find col on same s.poly as the last col
		bool found = false;
		foreachi(sCol &c, c_in, i)
			if (c.type == c.TYPE_OTHER_EDGE){
				for (int k=0;k<2;k++)
					if (s.Edge[c.edge].Polygon[k] == last_poly){
						//msg_write(format("%d  %d  - %d", c.type, c.polygon, c.edge));
						c_out.add(c);
						last_poly = s.Edge[c.edge].Polygon[1 - k];
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

void find_contours(ModelGeometry &m, ModelPolygon *t, ModelGeometry &s, Array<Array<sCol> > &c_out, bool inverse)
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
		if (cc.num < 3){
			for (int i=0;i<t->Side.num;i++)
				ed->multi_view_3d->AddMessage3d("p"+i2s(i), m.Vertex[t->Side[i].Vertex].pos);
			foreachi(sCol &c, cc, i)
				ed->multi_view_3d->AddMessage3d("x"+i2s(i), c.p);
			throw ActionException("contour with num<3");
		}
	}
}

void sort_and_join_contours(ModelGeometry &m, ModelPolygon *t, ModelGeometry &b, Array<Array<sCol> > &c_in, bool inverse)
{
	msg_db_f("sort_and_join_contours", 1);

	// find old vertices
	Array<sCol> v;
	for (int k=0;k<t->Side.num;k++){
		vector pos = m.Vertex[t->Side[k].Vertex].pos;
		if (b.IsInside(pos) == inverse)
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
}


float get_ang(Array<sCol> &c, int i, const vector &flat_n)
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

bool vertex_in_tria(sCol &a, sCol &b, sCol &c, sCol &v, float &slope)
{
	float f, g;
	GetBaryCentric(v.p, b.p, c.p, a.p, f, g);
	slope = f / g;
	return ((f > 0) && (g > 0) && (f + g < 1));
}

void combine_contours(Array<Array<sCol> > &c, int ca, int ia, int cb, int ib)
{
	Array<sCol> temp = c[ca].sub(ia, c[ca].num - ia) + c[ca].sub(0, ia);
	c[ca] = temp;

	// copy one point
	temp = c[cb].sub(ib, c[cb].num - ib) + c[cb].sub(0, ib + 1);
	c[ca].append(temp);

	c.erase(cb);
}

void triangulate_contours(ModelGeometry &m, ModelPolygon *t, Array<Array<sCol> > &contours)
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
		int inside_i = -1, inside_c;
		foreachi(Array<sCol> &c, contours, ci)
		for (int i=0;i<c.num;i++){
			int i_p1 = (i+1) % c.num;
			int i_p2 = (i+2) % c.num;
			float f = get_ang(c, i_p1, t->TempNormal);
			if (f < 0)
				continue;
			// cheat: ...
			float f_n = get_ang(c, i_p2, t->TempNormal);
			float f_l = get_ang(c, i, t->TempNormal);
			if (f_n >= 0)
				f += 0.001f / (f_n + 0.001f);
			if (f_l >= 0)
				f += 0.001f / (f_l + 0.001f);

			if (f > f_max){
				int cur_inside_i = -1;
				int cur_inside_c = -1;
				float cur_inside_slope = -1;
				// other vertices within this triangle?
				bool ok = true;
				foreachi(Array<sCol> &other, contours, cj)
				for (int j=0;j<other.num;j++){
					if (&c == &other)
						if ((j == i) || (j == i_p1) || (j == i_p2))
							continue;
					float slope;
					if (vertex_in_tria(c[i], c[i_p1], c[i_p2], other[j], slope)){
						if (&c == &other){
							ok = false;
							break;
						}
						if ((slope < cur_inside_slope) || (cur_inside_i < 0)){
							cur_inside_i = j;
							cur_inside_c = cj;
							cur_inside_slope = slope;
						}
					}
				}

				if (ok){
					f_max = f;
					i_max = i;
					c_max = ci;
					inside_i = cur_inside_i;
					inside_c = cur_inside_c;
				}
			}
		}

		if (i_max < 0){
			/*for (int i=0;i<contours.num;i++)
				for (int j=0;j<contours[i].num;j++){
					ed->multi_view_3d->AddMessage3d(format("%d:%d", i, j), contours[i][j].p);
				}*/
			throw ActionException("could not fill contours");
		}


		if (inside_i < 0){
			/*msg_write("--");
			msg_write(format("%d  %d", c_max, i_max));
			msg_write(format("%d %d %d", i_max, (i_max+1) % contours[c_max].num, (i_max+2) % contours[c_max].num));*/
			Array<sCol> tt;
			tt.add(contours[c_max][i_max]);
			tt.add(contours[c_max][(i_max+1) % contours[c_max].num]);
			tt.add(contours[c_max][(i_max+2) % contours[c_max].num]);
			output.add(tt);

			contours[c_max].erase((i_max+1) % contours[c_max].num);
			if (contours[c_max].num < 3)
				contours.erase(c_max);
		}else{
			/*msg_write("-- inside");
			msg_write(format("%d  %d   %d %d", c_max, i_max, inside_c, inside_i));*/
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

bool combine_polygons(Array<Array<sCol> > &c, int ia, int ib)
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

void simplify_filling(Array<Array<sCol> > &c)
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


void PolygonSubtract(ModelGeometry &a, ModelPolygon *t, int t_index, ModelGeometry &b, ModelGeometry &out, bool keep_inside)
{
	msg_db_f("PolygonSubtract", 0);
	bool inverse = keep_inside;

	msg_write("-----sub");
	msg_write(col.num);

	// find contours
	Array<Array<sCol> > contours;
	find_contours(a, t, b, contours, inverse);

	triangulate_contours(a, t, contours);

	simplify_filling(contours);

	SkinGeneratorMulti sg;
	sg.init_polygon(a.Vertex, *t);

	// create new surfaces
	foreach(Array<sCol> &c, contours){
		//if (inverse)
		//	c.reverse();

		// create contour vertices
		Array<int> vv;
		Array<vector> sv;
		for (int i=0;i<c.num;i++){
			vv.add(out.Vertex.num);
			out.AddVertex(c[i].p);
		}

		// skin vertices
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			for (int i=0;i<c.num;i++)
				if (c[i].type == sCol::TYPE_OLD_VERTEX)
					sv.add(t->Side[c[i].side].SkinVertex[l]);
				else
					sv.add(sg.get(c[i].p, l));

		// fill contour with polygons
		out.AddPolygon(vv, sv);
		out.Polygon.back().Material = t->Material;
	}
}

// out = a - b (just surface diff)
bool SurfaceSubtractUnary(ModelGeometry &a, ModelGeometry &b, ModelGeometry &out, bool keep_inside)
{
	msg_db_f("SurfSubtractUnary", 0);
	bool has_changes = false;

	out.Vertex = a.Vertex;

	// collide both surfaces and create additional polygons
	foreachi(ModelPolygon &p, a.Polygon, i)
		if (CollidePolygonSurface(a, &p, b, i)){
			PolygonSubtract(a, &p, i, b, out, keep_inside);
			has_changes = true;
		}else if (PolygonInsideSurface(a, &p, b) == keep_inside){
			ModelPolygon pp = p;
			out.Polygon.add(pp);
		}else{
			has_changes = true;
		}

	out.RemoveUnusedVertices();
	return has_changes;
}


// out = a - b
int ModelGeometrySubtract(ModelGeometry &a, ModelGeometry &b, ModelGeometry &out)
{
	msg_db_f("ModelGeometrySubtract", 0);

	a.UpdateTopology();
	b.UpdateTopology();
	foreach(ModelPolygon &p, a.Polygon)
		p.TempNormal = p.GetNormal(a.Vertex);
	foreach(ModelPolygon &p, b.Polygon)
		p.TempNormal = p.GetNormal(b.Vertex);
	if (!b.IsClosed)
		return -1;

	bool diff = false;

	//try{

	diff |= SurfaceSubtractUnary(a, b, out, false);

	if (a.IsClosed){
		ModelGeometry t;
		diff |= SurfaceSubtractUnary(b, a, t, true);
		t.Invert();
		out.Add(t);
	}

	/*}catch(ActionException &e){
		msg_error(e.message);
		return false;
	}*/


	vector min, max;
	out.GetBoundingBox(min, max);
	out.Weld((max - min).length() / 4000);

	return diff ? 1 : 0;
}

// out = a & b
int ModelGeometryAnd(ModelGeometry &a, ModelGeometry &b, ModelGeometry &out)
{
	msg_db_f("ModelGeometryAnd", 0);

	a.UpdateTopology();
	b.UpdateTopology();
	foreach(ModelPolygon &p, a.Polygon)
		p.TempNormal = p.GetNormal(a.Vertex);
	foreach(ModelPolygon &p, b.Polygon)
		p.TempNormal = p.GetNormal(b.Vertex);
	if (!b.IsClosed)
		return -1;

	bool diff = false;

	//try{

	diff |= SurfaceSubtractUnary(a, b, out, true);

	if (a.IsClosed){
		ModelGeometry t;
		diff |= SurfaceSubtractUnary(b, a, t, true);
		out.Add(t);
	}

	/*}catch(ActionException &e){
		msg_error(e.message);
		return false;
	}*/


	vector min, max;
	out.GetBoundingBox(min, max);
	out.Weld((max - min).length() / 4000);

	return diff ? 1 : 0;
}


