/*
 * ModeModelSurface.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelSurface.h"
#include "DataModel.h"

ModeModelSurface::ModeModelSurface()
{
}

ModeModelSurface::~ModeModelSurface()
{
}

void ModeModelSurface::AddVertex(int v)
{
	// set -> unique
	Vertex.add(v);

	// ref count
	model->Vertex[v].RefCount ++;

	// back reference
	model->Vertex[v].Surface = model->get_surf_no(this);
	if (model->Vertex[v].Surface < 0)
		msg_error("SurfaceAddVertex ...surface not found");
}

void ModeModelSurface::AddTriangle(int a, int b, int c, int material, const vector *sa, const vector *sb, const vector *sc, int index)
{
	msg_db_r("Surf.AddTria", 1);

	ModeModelTriangle t;
	t.Vertex[0] = a;
	t.Vertex[1] = b;
	t.Vertex[2] = c;
	for (int i=0;i<model->Material[material].NumTextures;i++){
		t.SkinVertex[i][0] = sa[i];
		t.SkinVertex[i][1] = sb[i];
		t.SkinVertex[i][2] = sc[i];
	}
	for (int k=0;k<3;k++){
		t.Edge[k] = AddEdgeForNewTriangle(t.Vertex[k], t.Vertex[(k + 1) % 3], Triangle.num, k);
		t.EdgeDirection[k] = Edge[t.Edge[k]].RefCount - 1;
	}
	AddVertex(a);
	AddVertex(b);
	AddVertex(c);

	// closed?
	UpdateClosed();

	t.is_selected = true;
	t.Material = material;
	t.view_stage = model->ViewStage;
	t.NormalDirty = true;
	if (index >= 0){
		Triangle.insert(t, index);

		// correct edges
		foreach(Edge, e)
			for (int k=0;k<e.RefCount;k++)
				if (e.Triangle[k] >= index)
					e.Triangle[k] ++;

		// correct own edges
		for (int k=0;k<3;k++)
			Edge[Triangle[index].Edge[k]].Triangle[Triangle[index].EdgeDirection[k]] = index;
	}else
		Triangle.add(t);
	msg_db_l(1);
}

int ModeModelSurface::AddEdgeForNewTriangle(int a, int b, int tria, int side)
{
	foreachi(Edge, e, i){
		if ((e.Vertex[0] == a) && (e.Vertex[1] == b)){
			e.RefCount ++;
			msg_error("surface error? inverse edge");
			e.Triangle[1] = tria;
			e.Side[1] = side;
			return i;
		}
		if ((e.Vertex[0] == b) && (e.Vertex[1] == a)){
			e.RefCount ++;
			if (e.RefCount > 2)
				msg_error("surface error? edge refcount > 2");
			e.Triangle[1] = tria;
			e.Side[1] = side;
			return i;
		}
	}
	ModeModelEdge ee;
	ee.Vertex[0] = a;
	ee.Vertex[1] = b;
	ee.is_selected = false;
	ee.is_special = false;
	ee.IsRound = false;
	ee.RefCount = 1;
	ee.Triangle[0] = tria;
	ee.Side[0] = side;
	ee.Triangle[1] = -1;
	Edge.add(ee);
	return Edge.num - 1;
}


inline bool edge_equal(ModeModelEdge *e, int a, int b)
{
	int v0 = e->Vertex[0];
	int v1 = e->Vertex[1];
	return (((v0 == a) && (v1 == b)) || ((v0 == b) && (v1 == a)));
}

inline int find_other_tria_from_edge(ModeModelSurface *s, int e, int t)
{
	if (s->Edge[e].Triangle[0] == t)
		return s->Edge[e].Triangle[1];
	return s->Edge[e].Triangle[0];
}

// return: closed circle... don't run again to the left
inline bool find_tria_top(ModeModelSurface *s, const Array<int> &ti, const Array<int> &tv, Set<int> &used, bool to_the_right)
{
	int t0 = 0;
	while(true){
		int ne = tv[t0];
		if (!to_the_right)
			ne = (ne + 2) % 3;
		int e = s->Triangle[ti[t0]].Edge[ne];
		if (!s->Edge[e].IsRound)
			return false;
		int tt = find_other_tria_from_edge(s, e, ti[t0]);
		if (tt < 0)
			return false;
		t0 = -1;
		for (int i=0;i<ti.num;i++)
			if (ti[i] == tt)
				t0 = i;
		if (t0 <= 0)
			return (t0 == 0);
		used.add(t0);
	}
}


void ModeModelSurface::UpdateClosed()
{
	// closed?
	IsClosed = true;
	foreach(Edge, e)
		if (e.RefCount != 2){
			IsClosed = false;
			break;
		}
}

void ModeModelSurface::RemoveObsoleteEdge(int index)
{
	msg_db_r("Surf.RemoveObsoleteEdge", 2);
	// correct triangle references
	foreach(Triangle, t)
		for (int k=0;k<3;k++)
			if (t.Edge[k] > index)
				t.Edge[k] --;
			else if (t.Edge[k] == index)
				msg_error(format("surf rm edge: edge not really obsolete  rc=%d (%d,%d) (%d,%d)", Edge[index].RefCount, t.Vertex[k], t.Vertex[(k+1)%3], Edge[index].Vertex[0], Edge[index].Vertex[1]));

	// delete
	Edge.erase(index);
	msg_db_l(2);
}

void ModeModelSurface::MergeEdges()
{
	msg_db_r("Surf.MergeEdges", 1);

	TestSanity("MergeEdges prae");

	foreachi(Edge, e, i){
		for (int j=i+1;j<Edge.num;j++){
			ModeModelEdge &f = Edge[j];
			if (edge_equal(&e, f.Vertex[0], f.Vertex[1])){
				if (e.RefCount + f.RefCount > 2)
					msg_error(format("SurfMergeEdges: edge(%d,%d).RefCount...  %d + %d    tria=(%d,%d,%d,%d)", f.Vertex[0], f.Vertex[1], e.RefCount, f.RefCount, e.Triangle[0], e.Triangle[1], f.Triangle[0], f.Triangle[1]));

				// add a link to the triangle
				e.RefCount ++;
				e.Triangle[1] = f.Triangle[0];
				e.Side[1] = f.Side[0];

				// relink triangles
				for (int k=0;k<3;k++)
					if (Triangle[e.Triangle[1]].Edge[k] == j){
						Triangle[e.Triangle[1]].Edge[k] = i;
						Triangle[e.Triangle[1]].EdgeDirection[k] = 1;
					}
				RemoveObsoleteEdge(j);
				break;
			}
		}
	}
	TestSanity("MergeEdges post");
	msg_db_l(1);
}

void ModeModelSurface::UpdateNormals()
{
	msg_db_r("Surf.UpdateNormals", 2);
	Set<int> edge, vert;

	// "flat" triangle normals
	foreach(Triangle, t)
		if (t.NormalDirty){
			t.NormalDirty = false;
			vector a = model->Vertex[t.Vertex[0]].pos;
			vector b = model->Vertex[t.Vertex[1]].pos;
			vector c = model->Vertex[t.Vertex[2]].pos;
			t.TempNormal = (b - a) ^ (c - a);
			VecNormalize(t.TempNormal);

			for (int k=0;k<3;k++)
				t.Normal[k] = t.TempNormal;

			foreachi(Edge, e, i)
				if (e.RefCount == 2){
					if (edge_equal(&e, t.Vertex[0], t.Vertex[1]))
						edge.add(i);
					if (edge_equal(&e, t.Vertex[1], t.Vertex[2]))
						edge.add(i);
					if (edge_equal(&e, t.Vertex[2], t.Vertex[0]))
						edge.add(i);
				}
		}

	// round edges?
	foreach(edge, ip){
		ModeModelEdge &e = Edge[ip];

		// adjoined triangles
		ModeModelTriangle &t1 = Triangle[e.Triangle[0]];
		ModeModelTriangle &t2 = Triangle[e.Triangle[1]];

		ModeModelVertex &v1 = model->Vertex[e.Vertex[0]];
		ModeModelVertex &v2 = model->Vertex[e.Vertex[1]];

		// round?
		e.IsRound = false;
		if ((v1.NormalMode == NormalModeAngular) || (v2.NormalMode == NormalModeAngular))
			e.IsRound = (fabs(t1.TempNormal * t2.TempNormal) > 0.5f);

		if (((v1.NormalMode == NormalModeAngular) && (e.IsRound)) || (v1.NormalMode == NormalModeSmooth))
			vert.add(e.Vertex[0]);
		if (((v2.NormalMode == NormalModeAngular) && (e.IsRound)) || (v2.NormalMode == NormalModeSmooth))
			vert.add(e.Vertex[1]);

		/*if (e.IsRound){
			vector n = t1.TempNormal + t2.TempNormal;
			VecNormalize(n);
			for (int k=0;k<3;k++)
				if ((t1.Vertex[k] == e.Vertex[0]) || (t1.Vertex[k] == e.Vertex[1]))
					t1.Normal[k] = n;
			for (int k=0;k<3;k++)
				if ((t2.Vertex[k] == e.Vertex[0]) || (t2.Vertex[k] == e.Vertex[1]))
					t2.Normal[k] = n;
		}*/
	}

	// per vertex...
	foreach(vert, ip){

		// hard vertex -> nothing to do
		if (model->Vertex[ip].NormalMode == NormalModeHard)
			continue;

		// find all triangles shared by this vertex
		Array<int> ti, tv;
		foreachi(Triangle, t, i){
			for (int k=0;k<3;k++){
				if (t.Vertex[k] == ip){
					t.Normal[k] = t.TempNormal;
					ti.add(i);
					tv.add(k);
				}
			}
		}

		// smooth vertex
		if (model->Vertex[ip].NormalMode == NormalModeSmooth){

			// average normal
			vector n = v0;
			for (int i=0;i<ti.num;i++)
				n += Triangle[ti[i]].Normal[tv[i]];
			VecNormalize(n);
			// apply normal...
			for (int i=0;i<ti.num;i++)
				Triangle[ti[i]].Normal[tv[i]] = n;
			continue;
		}

		// angular vertex...

		// find groups of triangles that are connected by round edges
		while (ti.num > 0){

			// start with the 1st triangle
			Set<int> used;
			used.add(0);

			// search to the right
			bool closed = find_tria_top(this, ti, tv, used, true);

			// search to the left
			if (!closed)
				find_tria_top(this, ti, tv, used, false);

			if (used.num == 1){
				// no smoothly connected triangles...
				ti.erase(0);
				tv.erase(0);
				continue;
			}

			// average normal
			vector n = v0;
			for (int i=0;i<used.num;i++)
				n += Triangle[ti[used[i]]].Normal[tv[used[i]]];
			VecNormalize(n);
			// apply normal... and remove from list
			for (int i=used.num-1;i>=0;i--){
				Triangle[ti[used[i]]].Normal[tv[used[i]]] = n;
				ti.erase(used[i]);
				tv.erase(used[i]);
			}
		}
	}
	msg_db_l(2);
}


void ModeModelSurface::BuildFromTriangles()
{
	// clear
	Edge.clear();
	Vertex.clear();
	int n = model->get_surf_no(this);
	foreach(model->Vertex, v)
		if (v.Surface == n){
			v.Surface = -1;
			v.RefCount = 0;
		}

	// add all triangles
	foreachi(Triangle, t, ti){
		// vertices
		for (int k=0;k<3;k++)
			AddVertex(t.Vertex[k]);

		// edges
		for (int k=0;k<3;k++){
			t.Edge[k] = AddEdgeForNewTriangle(t.Vertex[k], t.Vertex[(k + 1) % 3], ti, k);
			t.EdgeDirection[k] = Edge[t.Edge[k]].RefCount - 1;
		}
	}

	UpdateClosed();
}


void ModeModelSurface::RemoveTriangle(int index)
{
	ModeModelTriangle &t = Triangle[index];

	// unref the vertices
	for (int k=0;k<3;k++){
		model->Vertex[t.Vertex[k]].RefCount --;
		if (model->Vertex[t.Vertex[k]].RefCount == 0)
			model->Vertex[t.Vertex[k]].Surface = -1;
		Vertex.erase(t.Vertex[k]);
	}

	Set<int> obsolete;

	// remove from its 3 edges
	for (int k=0;k<3;k++){
		ModeModelEdge &e = Edge[t.Edge[k]];
		e.RefCount --;
		if (e.RefCount > 0){
			// edge has other triangle...
			if (t.EdgeDirection[k] > 0){
				e.Triangle[1] = -1;
			}else{
				// flip ownership
				e.Triangle[0] = e.Triangle[1];
				e.Side[0] = e.Side[1];
				e.Triangle[1] = -1;

				// swap vertices
				int v = e.Vertex[0];
				e.Vertex[0] = e.Vertex[1];
				e.Vertex[1] = v;

				// relink other triangle
				Triangle[e.Triangle[0]].EdgeDirection[e.Side[0]] = 0;
			}
		}else{
			e.Triangle[0] = -1;
			obsolete.add(t.Edge[k]);
		}
	}

	// correct edge links
	foreachi(Edge, e, i)
		for (int k=0;k<e.RefCount;k++)
			if (e.Triangle[k] > index)
				e.Triangle[k] --;
			else if (e.Triangle[k] == index){
				msg_error("RemoveTriangle: tria == index");
			}

	Triangle.erase(index);

	//TestSanity("rem tria 0");

	// remove obsolete edges
	foreachb(obsolete, o)
		RemoveObsoleteEdge(o);

	TestSanity("rem tria");
}

void ModeModelSurface::TestSanity(const string &loc)
{
	foreach(Triangle, t)
		if ((t.Vertex[0] == t.Vertex[1]) || (t.Vertex[1] == t.Vertex[2]) || (t.Vertex[2] == t.Vertex[0])){
			msg_error(loc + ": surf broken!   trivial tria");
			return;
		}
	foreachi(Edge, e, i){
		if (e.Vertex[0] == e.Vertex[1]){
			msg_error(loc + ": surf broken!   trivial edge");
			return;
		}
		for (int k=0;k<e.RefCount;k++)
			if (Triangle[e.Triangle[k]].Edge[e.Side[k]] != i){
				msg_error(loc + ": surf broken!   edge linkage");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.Side[k], Triangle[e.Triangle[k]].Edge[e.Side[k]], Triangle[e.Triangle[k]].EdgeDirection[e.Side[k]]));
				return;
			}

	}
}

bool ModeModelSurface::IsInside(const vector &p)
{
	if (!IsClosed)
		return false;

	// how often does a ray from p intersect the surface?
	int n = 0;
	foreach(Triangle, t){
		vector v[3];
		for (int k=0;k<3;k++)
			v[k] = model->Vertex[t.Vertex[k]].pos;

		// fast tests
		if ((v[0].x < p.x) && (v[1].x < p.x) && (v[2].x < p.x))
			continue;
		if (((v[0].y < p.y) == (v[1].y < p.y)) && ((v[0].y < p.y) == (v[2].y < p.y)))
			continue;
		if (((v[0].z < p.z) == (v[1].z < p.z)) && ((v[0].z < p.z) == (v[2].z < p.z)))
			continue;

		// real intersection
		vector col;
		if (LineIntersectsTriangle(v[0], v[1], v[2], p, p + e_x, col, false))
			if (col.x > p.x)
				n ++;
	}

	// even or odd?
	return ((n % 2) == 1);
}
