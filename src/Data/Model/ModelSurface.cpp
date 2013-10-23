/*
 * ModelSurface.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelSurface.h"
#include "DataModel.h"
#include "BspTree.h"
#include "../../Edward.h"



/*struct SurfaceInsideTestData
{
	int num_trias;
	Array<Ray> ray;
	Array<plane> pl;
};*/

struct SurfaceInsideTestData : BspTree{};

ModelSurface::ModelSurface()
{
	inside_data = NULL;
}

void ModelSurface::AddVertex(int v)
{
	int surf = model->get_surf_no(this);
	if (surf < 0)
		throw GeometryException("Surface.AddVertex: surface not part of the model??");
	if ((model->Vertex[v].Surface != surf) && (model->Vertex[v].Surface >= 0))
		throw GeometryException("Surface.AddVertex: vertex already part of an other surface");

	// set -> unique
	Vertex.add(v);

	// ref count
	model->Vertex[v].RefCount ++;

	// back reference
	model->Vertex[v].Surface = surf;
}

bool int_array_has_duplicates(Array<int> &a)
{
	for (int i=0; i<a.num; i++)
		for (int j=i+1; j<a.num; j++)
			if (a[i] == a[j])
				return true;
	return false;
}

void ModelSurface::AddPolygon(Array<int> &v, int material, Array<vector> &sv, int index)
{
	if (int_array_has_duplicates(v))
		throw GeometryException("AddPolygon: duplicate vertices");

	msg_db_f("Surf.AddTria", 1);

	ModelPolygon t;
	t.Side.resize(v.num);
	for (int k=0;k<v.num;k++){
		t.Side[k].Vertex = v[k];
		for (int i=0;i<model->Material[material].NumTextures;i++)
			t.Side[k].SkinVertex[i] = sv[i * v.num + k];
	}
	for (int k=0;k<v.num;k++){
		try{
			t.Side[k].Edge = AddEdgeForNewPolygon(t.Side[k].Vertex, t.Side[(k + 1) % v.num].Vertex, Polygon.num, k);
			t.Side[k].EdgeDirection = Edge[t.Side[k].Edge].RefCount - 1;
		}catch(GeometryException &e){
			// failed -> clean up
			for (int i=Edge.num-1;i>=0;i--)
				for (int j=0;j<Edge[i].RefCount;j++)
					if (Edge[i].Polygon[j] == Polygon.num){
						Edge[i].RefCount --;
						if (Edge[i].RefCount == 0)
							Edge.resize(i);
					}
			throw(e);
		}
	}
	for (int k=0;k<v.num;k++)
		AddVertex(v[k]);

	// closed?
	UpdateClosed();

	t.is_selected = false;
	t.Material = material;
	t.view_stage = ed->multi_view_3d->view_stage;
	t.NormalDirty = true;
	t.TriangulationDirty = true;
	if (index >= 0){
		Polygon.insert(t, index);

		// correct edges
		foreach(ModelEdge &e, Edge)
			for (int k=0;k<e.RefCount;k++)
				if (e.Polygon[k] >= index)
					e.Polygon[k] ++;

		// correct own edges
		for (int k=0;k<t.Side.num;k++)
			Edge[Polygon[index].Side[k].Edge].Polygon[Polygon[index].Side[k].EdgeDirection] = index;
	}else
		Polygon.add(t);
}

int ModelSurface::AddEdgeForNewPolygon(int a, int b, int tria, int side)
{
	foreachi(ModelEdge &e, Edge, i){
		if ((e.Vertex[0] == a) && (e.Vertex[1] == b)){
			throw GeometryException("the new polygon would have neighbors of opposite orientation");
			/*e.RefCount ++;
			msg_error("surface error? inverse edge");
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;*/
		}
		if ((e.Vertex[0] == b) && (e.Vertex[1] == a)){
			if (e.Polygon[0] == tria)
				throw GeometryException("the new polygon would contain the same edge twice");
			if (e.RefCount > 1)
				throw GeometryException("there would be more than 2 polygons sharing an egde");
			e.RefCount ++;
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;
		}
	}
	ModelEdge ee;
	ee.Vertex[0] = a;
	ee.Vertex[1] = b;
	ee.is_selected = false;
	ee.is_special = false;
	ee.IsRound = false;
	ee.RefCount = 1;
	ee.Polygon[0] = tria;
	ee.Side[0] = side;
	ee.Polygon[1] = -1;
	Edge.add(ee);
	return Edge.num - 1;
}


inline bool edge_equal(ModelEdge &e, int a, int b)
{
	int v0 = e.Vertex[0];
	int v1 = e.Vertex[1];
	return (((v0 == a) && (v1 == b)) || ((v0 == b) && (v1 == a)));
}

inline int find_other_tria_from_edge(ModelSurface *s, int e, int t)
{
	if (s->Edge[e].Polygon[0] == t)
		return s->Edge[e].Polygon[1];
	return s->Edge[e].Polygon[0];
}


struct PolySideData
{
	int poly;
	int side;
};

// return: closed circle... don't run again to the left
inline bool find_tria_top(ModelSurface *s, const Array<PolySideData> &pd, Set<int> &used, bool to_the_right)
{
	int t0 = 0;
	while(true){
		int side = pd[t0].side;
		if (!to_the_right){
			int ns = s->Polygon[pd[t0].poly].Side.num;
			side = (side + ns - 1) % ns;
		}
		int e = s->Polygon[pd[t0].poly].Side[side].Edge;
		if (!s->Edge[e].IsRound)
			return false;
		int tt = find_other_tria_from_edge(s, e, pd[t0].poly);
		if (tt < 0)
			return false;
		t0 = -1;
		for (int i=0;i<pd.num;i++)
			if (pd[i].poly == tt)
				t0 = i;
		if (t0 <= 0)
			return (t0 == 0);
		used.add(t0);
	}
}


void ModelSurface::UpdateClosed()
{
	// closed?
	IsClosed = true;
	foreach(ModelEdge &e, Edge)
		if (e.RefCount != 2){
			IsClosed = false;
			break;
		}
}

void ModelSurface::RemoveObsoleteEdge(int index)
{
	msg_db_f("Surf.RemoveObsoleteEdge", 2);
	// correct triangle references
	foreach(ModelPolygon &t, Polygon)
		for (int k=0;k<t.Side.num;k++)
			if (t.Side[k].Edge > index)
				t.Side[k].Edge --;
			else if (t.Side[k].Edge == index)
				msg_error(format("surf rm edge: edge not really obsolete  rc=%d (%d,%d) (%d,%d)", Edge[index].RefCount, t.Side[k].Vertex, t.Side[(k+1)%t.Side.num].Vertex, Edge[index].Vertex[0], Edge[index].Vertex[1]));

	// delete
	Edge.erase(index);
}

void ModelSurface::MergeEdges()
{
	msg_db_f("Surf.MergeEdges", 1);

	TestSanity("MergeEdges prae");

	foreachi(ModelEdge &e, Edge, i){
		for (int j=i+1;j<Edge.num;j++){
			ModelEdge &f = Edge[j];
			if (edge_equal(e, f.Vertex[0], f.Vertex[1])){
				if (e.RefCount + f.RefCount > 2)
					msg_error(format("SurfMergeEdges: edge(%d,%d).RefCount...  %d + %d    tria=(%d,%d,%d,%d)", f.Vertex[0], f.Vertex[1], e.RefCount, f.RefCount, e.Polygon[0], e.Polygon[1], f.Polygon[0], f.Polygon[1]));

				// add a link to the triangle
				e.RefCount ++;
				e.Polygon[1] = f.Polygon[0];
				e.Side[1] = f.Side[0];

				// relink triangles
				ModelPolygon &poly = Polygon[e.Polygon[1]];
				for (int k=0;k<poly.Side.num;k++)
					if (poly.Side[k].Edge == j){
						poly.Side[k].Edge = i;
						poly.Side[k].EdgeDirection = 1;
					}
				RemoveObsoleteEdge(j);
				break;
			}
		}
	}
	TestSanity("MergeEdges post");
}

void ModelSurface::UpdateNormals()
{
	msg_db_f("Surf.UpdateNormals", 2);
	Set<int> edge, vert;

	// "flat" triangle normals
	foreach(ModelPolygon &t, Polygon)
		if (t.NormalDirty){
			t.NormalDirty = false;

			t.TempNormal = t.GetNormal(model->Vertex);

			for (int k=0;k<t.Side.num;k++){
				t.Side[k].Normal = t.TempNormal;
				int e = t.Side[k].Edge;
				if (Edge[e].RefCount == 2)
					edge.add(e);
			}
		}

	// round edges?
	foreach(int ip, edge){
		ModelEdge &e = Edge[ip];

		// adjoined triangles
		ModelPolygon &t1 = Polygon[e.Polygon[0]];
		ModelPolygon &t2 = Polygon[e.Polygon[1]];

		ModelVertex &v1 = model->Vertex[e.Vertex[0]];
		ModelVertex &v2 = model->Vertex[e.Vertex[1]];

		// round?
		e.IsRound = false;
		if ((v1.NormalMode == NormalModeAngular) || (v2.NormalMode == NormalModeAngular))
			e.IsRound = (t1.TempNormal * t2.TempNormal > 0.6f);

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

	// find all triangles shared by each found vertex
	Array<Array<PolySideData> > poly_side;
	poly_side.resize(vert.num);
	foreachi(ModelPolygon &t, Polygon, i){
		for (int k=0;k<t.Side.num;k++){
			int n = vert.find(t.Side[k].Vertex);
			if (n >= 0){
				t.Side[k].Normal = t.TempNormal;
				PolySideData d;
				d.poly = i;
				d.side = k;
				poly_side[n].add(d);
			}
		}
	}

	// per vertex...
	foreachi(int ip, vert, nn){

		// hard vertex -> nothing to do
		if (model->Vertex[ip].NormalMode == NormalModeHard)
			continue;

		Array<PolySideData> &pd = poly_side[nn];

		// smooth vertex
		if (model->Vertex[ip].NormalMode == NormalModeSmooth){

			// average normal
			vector n = v_0;
			for (int i=0;i<pd.num;i++)
				n += Polygon[pd[i].poly].Side[pd[i].side].Normal;
			n.normalize();
			// apply normal...
			for (int i=0;i<pd.num;i++)
				Polygon[pd[i].poly].Side[pd[i].side].Normal = n;
			continue;
		}

		// angular vertex...

		// find groups of triangles that are connected by round edges
		while (pd.num > 0){

			// start with the 1st triangle
			Set<int> used;
			used.add(0);

			// search to the right
			bool closed = find_tria_top(this, pd, used, true);

			// search to the left
			if (!closed)
				find_tria_top(this, pd, used, false);

			if (used.num == 1){
				// no smoothly connected triangles...
				pd.erase(0);
				continue;
			}

			// average normal
			vector n = v_0;
			for (int i=0;i<used.num;i++)
				n += Polygon[pd[used[i]].poly].Side[pd[used[i]].side].Normal;
			n.normalize();
			// apply normal... and remove from list
			for (int i=used.num-1;i>=0;i--){
				Polygon[pd[used[i]].poly].Side[pd[used[i]].side].Normal = n;
				pd.erase(used[i]);
			}
		}
	}
}


void ModelSurface::BuildFromPolygons()
{
	// clear
	Edge.clear();
	Vertex.clear();
	int n = model->get_surf_no(this);
	foreach(ModelVertex &v, model->Vertex)
		if (v.Surface == n){
			v.Surface = -1;
			v.RefCount = 0;
		}

	// add all triangles
	foreachi(ModelPolygon &t, Polygon, ti){
		// vertices
		for (int k=0;k<t.Side.num;k++)
			AddVertex(t.Side[k].Vertex);

		// edges
		for (int k=0;k<t.Side.num;k++){
			t.Side[k].Edge = AddEdgeForNewPolygon(t.Side[k].Vertex, t.Side[(k + 1) % t.Side.num].Vertex, ti, k);
			t.Side[k].EdgeDirection = Edge[t.Side[k].Edge].RefCount - 1;
		}
	}

	UpdateClosed();
}


void ModelSurface::RemovePolygon(int index)
{
	ModelPolygon &t = Polygon[index];

	// unref the vertices
	for (int k=0;k<t.Side.num;k++){
		model->Vertex[t.Side[k].Vertex].RefCount --;
		if (model->Vertex[t.Side[k].Vertex].RefCount == 0){
			model->Vertex[t.Side[k].Vertex].Surface = -1;
			Vertex.erase(t.Side[k].Vertex);
		}
	}

	Set<int> obsolete;

	// remove from its edges
	for (int k=0;k<t.Side.num;k++){
		ModelEdge &e = Edge[t.Side[k].Edge];
		e.RefCount --;
		if (e.RefCount > 0){
			// edge has other triangle...
			if (t.Side[k].EdgeDirection > 0){
				e.Polygon[1] = -1;
			}else{
				// flip ownership
				e.Polygon[0] = e.Polygon[1];
				e.Side[0] = e.Side[1];
				e.Polygon[1] = -1;

				// swap vertices
				int v = e.Vertex[0];
				e.Vertex[0] = e.Vertex[1];
				e.Vertex[1] = v;

				// relink other triangle
				Polygon[e.Polygon[0]].Side[e.Side[0]].EdgeDirection = 0;
			}
		}else{
			e.Polygon[0] = -1;
			obsolete.add(t.Side[k].Edge);
		}
	}

	// correct edge links
	foreachi(ModelEdge &e, Edge, i)
		for (int k=0;k<e.RefCount;k++)
			if (e.Polygon[k] > index)
				e.Polygon[k] --;
			else if (e.Polygon[k] == index){
				throw GeometryException("RemoveTriangle: tria == index");
			}

	Polygon.erase(index);

	//TestSanity("rem poly 0");

	// remove obsolete edges
	foreachb(int o, obsolete)
		RemoveObsoleteEdge(o);

/*	if (!TestSanity("rem poly"))
		throw GeometryException("RemoveTriangle: TestSanity failed");*/
}

bool ModelSurface::TestSanity(const string &loc)
{
	foreach(ModelPolygon &t, Polygon)
		for (int k=0;k<t.Side.num;k++)
			for (int kk=k+1;kk<t.Side.num;kk++)
				if (t.Side[k].Vertex == t.Side[kk].Vertex){
					msg_error(loc + ": surf broken!   identical vertices in poly");
					return false;
				}
	foreachi(ModelEdge &e, Edge, i){
		if (e.Vertex[0] == e.Vertex[1]){
			msg_error(loc + ": surf broken!   trivial edge");
			return false;
		}
		for (int k=0;k<e.RefCount;k++){
			ModelPolygon &t = Polygon[e.Polygon[k]];
			if (t.Side[e.Side[k]].Edge != i){
				msg_error(loc + ": surf broken!   edge linkage");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.Side[k], t.Side[e.Side[k]].Edge, t.Side[e.Side[k]].EdgeDirection));
				return false;
			}
			if (t.Side[e.Side[k]].EdgeDirection != k){
				msg_error(loc + ": surf broken!   edge linkage (dir)");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.Side[k], t.Side[e.Side[k]].Edge, t.Side[e.Side[k]].EdgeDirection));
				return false;
			}
			for (int j=0;j<2;j++)
				if (e.Vertex[(j + k) % 2] != t.Side[(e.Side[k] + j) % t.Side.num].Vertex){
					msg_error(loc + ": surf broken!   edge linkage (vert)");
					msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.Side[k], t.Side[e.Side[k]].Edge, t.Side[e.Side[k]].EdgeDirection));
					return false;
				}
		}

	}
	return true;
}

bool ModelSurface::IsInside(const vector &p)
{
	if (!IsClosed)
		return false;
	BeginInsideTests();
	bool r = InsideTest(p);
	EndInsideTests();
	return r;
}

void ModelSurface::BeginInsideTests()
{
	if (!IsClosed)
		return;
	inside_data = new SurfaceInsideTestData;
	/*inside_data->num_trias = 0;
	foreach(ModelPolygon &t, Polygon)
		inside_data->num_trias += (t.Side.num - 2);
	inside_data->ray.resize(inside_data->num_trias * 3);
	inside_data->pl.resize(inside_data->num_trias);
	Ray *r = &inside_data->ray[0];
	plane *pl = &inside_data->pl[0];
	foreach(ModelPolygon &t, Polygon){
		if (t.TriangulationDirty)
			t.UpdateTriangulation(model->Vertex);
		for (int k=0;k<t.Side.num-2;k++){
			*(pl ++) = plane(model->Vertex[t.Side[0].Vertex].pos, t.TempNormal);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[0]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[1]].Vertex].pos);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[1]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[2]].Vertex].pos);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[2]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[0]].Vertex].pos);
		}
	}*/

	float epsilon = model->GetRadius() * 0.001f;
	foreach(ModelPolygon &p, Polygon)
		inside_data->add(p, model, epsilon);
}

void ModelSurface::EndInsideTests()
{
	if (inside_data)
		delete(inside_data);
}

inline bool ray_intersect_tria(Ray &r, Ray *pr, plane *pl, vector &cp)
{
	bool r0 = (r.dot(pr[0]) > 0);
	bool r1 = (r.dot(pr[1]) > 0);
	if (r1 != r0)
		return false;
	bool r2 = (r.dot(pr[2]) > 0);
	if (r2 != r0)
		return false;

	return r.intersect_plane(*pl, cp);
}

bool ModelSurface::InsideTest(const vector &p)
{
	if (!inside_data)
		return false;
	return inside_data->inside(p);

	/*
	Ray r = Ray(p, p + e_x);

	// how often does a ray from p intersect the surface?
	int n = 0;
	Ray *pr = &inside_data->ray[0];
	Ray *pr_end = pr + inside_data->num_trias * 3;
	plane *pl = &inside_data->pl[0];
	for (;pr < pr_end; pr += 3, pl ++){

		// plane test
		if ((p * pl->n + pl->d  > 0) == (pl->n.x > 0))
			continue;

		vector cp;
		if (!ray_intersect_tria(r, pr, pl, cp))
			continue;

		if (cp.x > p.x)
			n ++;
	}*/


	/*Array<vector> v;
	int n = 0;
	foreach(ModelPolygon &t, Polygon){

		// plane test
		if (((p - model->Vertex[t.Side[0].Vertex].pos) * t.TempNormal > 0) == (t.TempNormal.x > 0))
			continue;

		// polygon data
		if (v.num < t.Side.num)
			v.resize(t.Side.num);
		for (int k=0;k<t.Side.num;k++)
			v[k] = model->Vertex[t.Side[k].Vertex].pos;

		// bounding box tests
		bool smaller = true;
		for (int k=0;k<t.Side.num;k++)
			if (v[k].x >= p.x)
				smaller = false;
		if (smaller)
			continue;

		smaller = true;
		for (int k=1;k<t.Side.num;k++){
			if ((v[0].y < p.y) !=  (v[k].y < p.y))
				smaller = false;
			if ((v[0].z < p.z) !=  (v[k].z < p.z))
				smaller = false;
		}
		if (smaller)
			continue;

		// real intersection
		vector col;
		if (t.TriangulationDirty)
			t.UpdateTriangulation(model->Vertex);
		for (int k=t.Side.num-2;k>=0;k--)
			if (LineIntersectsTriangle(v[t.Side[k].Triangulation[0]], v[t.Side[k].Triangulation[1]], v[t.Side[k].Triangulation[2]], p, p + e_x, col, false))
				if (col.x > p.x)
					n ++;
	}*/

	// even or odd?
	//return ((n % 2) == 1);
}



Array<int> ModelSurface::GetBoundaryLoop(int v0)
{
	Array<int> loop;
	int last = v0;
	bool found = true;
	while(found){
		found = false;
		foreach(ModelEdge &e, Edge)
			if (e.RefCount == 1)
				if (e.Vertex[0] == last){
					last = e.Vertex[1];
					loop.add(last);
					if (last == v0)
						return loop;
					found = true;
					break;
				}
	}
	return loop;
}

int ModelSurface::FindEdge(int vertex0, int vertex1)
{
	foreachi(ModelEdge &e, Edge, i)
		if (((e.Vertex[0] == vertex0) && (e.Vertex[1] == vertex1)) || ((e.Vertex[1] == vertex0) && (e.Vertex[0] == vertex1)))
			return i;
	return -1;
}

Array<Array<int> > ModelSurface::GetConnectedComponents()
{
	Array<Array<int> > cc;
	return cc;
}
