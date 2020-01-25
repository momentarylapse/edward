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
#include "../../MultiView/MultiView.h"



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
	model = NULL;
	is_closed = false;
	is_visible = true;
	is_physical = false;
}

void ModelSurface::addVertex(int v)
{
	int surf = model->get_surf_no(this);
	if (surf < 0)
		throw GeometryException("Surface.AddVertex: surface not part of the model??");
	if ((model->vertex[v].surface != surf) && (model->vertex[v].surface >= 0))
		throw GeometryException("Surface.AddVertex: vertex already part of an other surface");

	// set -> unique
	vertex.add(v);

	// ref count
	model->vertex[v].ref_count ++;

	// back reference
	model->vertex[v].surface = surf;
}

bool int_array_has_duplicates(const Array<int> &a)
{
	for (int i=0; i<a.num; i++)
		for (int j=i+1; j<a.num; j++)
			if (a[i] == a[j])
				return true;
	return false;
}

void ModelSurface::addPolygon(const Array<int> &v, int material, const Array<vector> &sv, int index)
{
	if (int_array_has_duplicates(v))
		throw GeometryException("AddPolygon: duplicate vertices");

	ModelPolygon t;
	t.side.resize(v.num);
	for (int k=0;k<v.num;k++){
		t.side[k].vertex = v[k];
		for (int i=0;i<model->material[material]->texture_levels.num;i++)
			t.side[k].skin_vertex[i] = sv[i * v.num + k];
	}
	for (int k=0;k<v.num;k++){
		try{
			t.side[k].edge = addEdgeForNewPolygon(t.side[k].vertex, t.side[(k + 1) % v.num].vertex, polygon.num, k);
			t.side[k].edge_direction = edge[t.side[k].edge].ref_count - 1;
		}catch(GeometryException &e){
			// failed -> clean up
			for (int i=edge.num-1;i>=0;i--)
				for (int j=0;j<edge[i].ref_count;j++)
					if (edge[i].polygon[j] == polygon.num){
						edge[i].ref_count --;
						if (edge[i].ref_count == 0)
							edge.resize(i);
					}
			throw(e);
		}
	}
	for (int k=0;k<v.num;k++)
		addVertex(v[k]);

	// closed?
	updateClosed();

	t.is_selected = false;
	t.material = material;
	t.view_stage = ed->multi_view_3d->view_stage;
	t.normal_dirty = true;
	t.triangulation_dirty = true;
	if (index >= 0){
		polygon.insert(t, index);

		// correct edges
		for (ModelEdge &e: edge)
			for (int k=0;k<e.ref_count;k++)
				if (e.polygon[k] >= index)
					e.polygon[k] ++;

		// correct own edges
		for (int k=0;k<t.side.num;k++)
			edge[polygon[index].side[k].edge].polygon[polygon[index].side[k].edge_direction] = index;
	}else
		polygon.add(t);
}

int ModelSurface::addEdgeForNewPolygon(int a, int b, int tria, int side)
{
	foreachi(ModelEdge &e, edge, i){
		if ((e.vertex[0] == a) && (e.vertex[1] == b)){
			throw GeometryException("the new polygon would have neighbors of opposite orientation");
			/*e.RefCount ++;
			msg_error("surface error? inverse edge");
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;*/
		}
		if ((e.vertex[0] == b) && (e.vertex[1] == a)){
			if (e.polygon[0] == tria)
				throw GeometryException("the new polygon would contain the same edge twice");
			if (e.ref_count > 1)
				throw GeometryException("there would be more than 2 polygons sharing an egde");
			e.ref_count ++;
			e.polygon[1] = tria;
			e.side[1] = side;
			return i;
		}
	}
	ModelEdge ee;
	ee.vertex[0] = a;
	ee.vertex[1] = b;
	ee.is_selected = false;
	ee.is_special = false;
	ee.is_round = false;
	ee.ref_count = 1;
	ee.polygon[0] = tria;
	ee.side[0] = side;
	ee.polygon[1] = -1;
	edge.add(ee);
	return edge.num - 1;
}


inline bool edge_equal(ModelEdge &e, int a, int b)
{
	int v0 = e.vertex[0];
	int v1 = e.vertex[1];
	return (((v0 == a) && (v1 == b)) || ((v0 == b) && (v1 == a)));
}

inline int find_other_tria_from_edge(ModelSurface *s, int e, int t)
{
	if (s->edge[e].polygon[0] == t)
		return s->edge[e].polygon[1];
	return s->edge[e].polygon[0];
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
			int ns = s->polygon[pd[t0].poly].side.num;
			side = (side + ns - 1) % ns;
		}
		int e = s->polygon[pd[t0].poly].side[side].edge;
		if (!s->edge[e].is_round)
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


void ModelSurface::updateClosed()
{
	// closed?
	is_closed = true;
	for (ModelEdge &e: edge)
		if (e.ref_count != 2){
			is_closed = false;
			break;
		}
}

void ModelSurface::removeObsoleteEdge(int index)
{
	// correct triangle references
	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].edge > index)
				t.side[k].edge --;
			else if (t.side[k].edge == index)
				msg_error(format("surf rm edge: edge not really obsolete  rc=%d (%d,%d) (%d,%d)", edge[index].ref_count, t.side[k].vertex, t.side[(k+1)%t.side.num].vertex, edge[index].vertex[0], edge[index].vertex[1]));

	// delete
	edge.erase(index);
}

void ModelSurface::mergeEdges()
{
	testSanity("MergeEdges prae");

	foreachi(ModelEdge &e, edge, i){
		for (int j=i+1;j<edge.num;j++){
			ModelEdge &f = edge[j];
			if (edge_equal(e, f.vertex[0], f.vertex[1])){
				if (e.ref_count + f.ref_count > 2)
					msg_error(format("SurfMergeEdges: edge(%d,%d).RefCount...  %d + %d    tria=(%d,%d,%d,%d)", f.vertex[0], f.vertex[1], e.ref_count, f.ref_count, e.polygon[0], e.polygon[1], f.polygon[0], f.polygon[1]));

				// add a link to the triangle
				e.ref_count ++;
				e.polygon[1] = f.polygon[0];
				e.side[1] = f.side[0];

				// relink triangles
				ModelPolygon &poly = polygon[e.polygon[1]];
				for (int k=0;k<poly.side.num;k++)
					if (poly.side[k].edge == j){
						poly.side[k].edge = i;
						poly.side[k].edge_direction = 1;
					}
				removeObsoleteEdge(j);
				break;
			}
		}
	}
	testSanity("MergeEdges post");
}

void ModelSurface::updateNormals()
{
	Set<int> ee, vert;

	// "flat" triangle normals
	for (ModelPolygon &t: polygon)
		if (t.normal_dirty){
			t.normal_dirty = false;

			t.temp_normal = t.getNormal(model->vertex);

			for (int k=0;k<t.side.num;k++){
				t.side[k].normal = t.temp_normal;
				int e = t.side[k].edge;
				if (edge[e].ref_count == 2)
					ee.add(e);
			}
		}

	// round edges?
	for (int ip: ee){
		ModelEdge &e = edge[ip];

		// adjoined triangles
		ModelPolygon &t1 = polygon[e.polygon[0]];
		ModelPolygon &t2 = polygon[e.polygon[1]];

		ModelVertex &v1 = model->vertex[e.vertex[0]];
		ModelVertex &v2 = model->vertex[e.vertex[1]];

		// round?
		e.is_round = false;
		if ((v1.normal_mode == NORMAL_MODE_ANGULAR) || (v2.normal_mode == NORMAL_MODE_ANGULAR))
			e.is_round = (t1.temp_normal * t2.temp_normal > 0.6f);

		if (((v1.normal_mode == NORMAL_MODE_ANGULAR) && (e.is_round)) || (v1.normal_mode == NORMAL_MODE_SMOOTH))
			vert.add(e.vertex[0]);
		if (((v2.normal_mode == NORMAL_MODE_ANGULAR) && (e.is_round)) || (v2.normal_mode == NORMAL_MODE_SMOOTH))
			vert.add(e.vertex[1]);

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
	foreachi(ModelPolygon &t, polygon, i){
		for (int k=0;k<t.side.num;k++){
			int n = vert.find(t.side[k].vertex);
			if (n >= 0){
				t.side[k].normal = t.temp_normal;
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
		if (model->vertex[ip].normal_mode == NORMAL_MODE_HARD)
			continue;

		Array<PolySideData> &pd = poly_side[nn];

		// smooth vertex
		if (model->vertex[ip].normal_mode == NORMAL_MODE_SMOOTH){

			// average normal
			vector n = v_0;
			for (int i=0;i<pd.num;i++)
				n += polygon[pd[i].poly].side[pd[i].side].normal;
			n.normalize();
			// apply normal...
			for (int i=0;i<pd.num;i++)
				polygon[pd[i].poly].side[pd[i].side].normal = n;
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
				n += polygon[pd[used[i]].poly].side[pd[used[i]].side].normal;
			n.normalize();
			// apply normal... and remove from list
			for (int i=used.num-1;i>=0;i--){
				polygon[pd[used[i]].poly].side[pd[used[i]].side].normal = n;
				pd.erase(used[i]);
			}
		}
	}
}


void ModelSurface::buildFromPolygons()
{
	// clear
	edge.clear();
	vertex.clear();
	int n = model->get_surf_no(this);
	for (ModelVertex &v: model->vertex)
		if (v.surface == n){
			v.surface = -1;
			v.ref_count = 0;
		}

	// add all triangles
	foreachi(ModelPolygon &t, polygon, ti){
		// vertices
		for (int k=0;k<t.side.num;k++)
			addVertex(t.side[k].vertex);

		// edges
		for (int k=0;k<t.side.num;k++){
			t.side[k].edge = addEdgeForNewPolygon(t.side[k].vertex, t.side[(k + 1) % t.side.num].vertex, ti, k);
			t.side[k].edge_direction = edge[t.side[k].edge].ref_count - 1;
		}
	}

	updateClosed();
}


void ModelSurface::removePolygon(int index)
{
	ModelPolygon &t = polygon[index];

	// unref the vertices
	for (int k=0;k<t.side.num;k++){
		model->vertex[t.side[k].vertex].ref_count --;
		if (model->vertex[t.side[k].vertex].ref_count == 0){
			model->vertex[t.side[k].vertex].surface = -1;
			vertex.erase(t.side[k].vertex);
		}
	}

	Set<int> obsolete;

	// remove from its edges
	for (int k=0;k<t.side.num;k++){
		ModelEdge &e = edge[t.side[k].edge];
		e.ref_count --;
		if (e.ref_count > 0){
			// edge has other triangle...
			if (t.side[k].edge_direction > 0){
				e.polygon[1] = -1;
			}else{
				// flip ownership
				e.polygon[0] = e.polygon[1];
				e.side[0] = e.side[1];
				e.polygon[1] = -1;

				// swap vertices
				int v = e.vertex[0];
				e.vertex[0] = e.vertex[1];
				e.vertex[1] = v;

				// relink other triangle
				polygon[e.polygon[0]].side[e.side[0]].edge_direction = 0;
			}
		}else{
			e.polygon[0] = -1;
			obsolete.add(t.side[k].edge);
		}
	}

	// correct edge links
	foreachi(ModelEdge &e, edge, i)
		for (int k=0;k<e.ref_count;k++)
			if (e.polygon[k] > index)
				e.polygon[k] --;
			else if (e.polygon[k] == index){
				throw GeometryException("RemoveTriangle: tria == index");
			}

	polygon.erase(index);

	//TestSanity("rem poly 0");

	// remove obsolete edges
	foreachb(int o, obsolete)
		removeObsoleteEdge(o);

/*	if (!TestSanity("rem poly"))
		throw GeometryException("RemoveTriangle: TestSanity failed");*/
}

bool ModelSurface::testSanity(const string &loc)
{
	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			for (int kk=k+1;kk<t.side.num;kk++)
				if (t.side[k].vertex == t.side[kk].vertex){
					msg_error(loc + ": surf broken!   identical vertices in poly");
					return false;
				}
	foreachi(ModelEdge &e, edge, i){
		if (e.vertex[0] == e.vertex[1]){
			msg_error(loc + ": surf broken!   trivial edge");
			return false;
		}
		for (int k=0;k<e.ref_count;k++){
			ModelPolygon &t = polygon[e.polygon[k]];
			if (t.side[e.side[k]].edge != i){
				msg_error(loc + ": surf broken!   edge linkage");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.side[k], t.side[e.side[k]].edge, t.side[e.side[k]].edge_direction));
				return false;
			}
			if (t.side[e.side[k]].edge_direction != k){
				msg_error(loc + ": surf broken!   edge linkage (dir)");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.side[k], t.side[e.side[k]].edge, t.side[e.side[k]].edge_direction));
				return false;
			}
			for (int j=0;j<2;j++)
				if (e.vertex[(j + k) % 2] != t.side[(e.side[k] + j) % t.side.num].vertex){
					msg_error(loc + ": surf broken!   edge linkage (vert)");
					msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.side[k], t.side[e.side[k]].edge, t.side[e.side[k]].edge_direction));
					return false;
				}
		}

	}
	return true;
}

bool ModelSurface::isInside(const vector &p)
{
	if (!is_closed)
		return false;
	beginInsideTests();
	bool r = insideTest(p);
	endInsideTests();
	return r;
}

void ModelSurface::beginInsideTests()
{
	if (!is_closed)
		return;
	inside_data = new SurfaceInsideTestData;
	/*inside_data->num_trias = 0;
	for (ModelPolygon &t, Polygon)
		inside_data->num_trias += (t.Side.num - 2);
	inside_data->ray.resize(inside_data->num_trias * 3);
	inside_data->pl.resize(inside_data->num_trias);
	Ray *r = &inside_data->ray[0];
	plane *pl = &inside_data->pl[0];
	for (ModelPolygon &t, Polygon){
		if (t.TriangulationDirty)
			t.UpdateTriangulation(model->Vertex);
		for (int k=0;k<t.Side.num-2;k++){
			*(pl ++) = plane(model->Vertex[t.Side[0].Vertex].pos, t.TempNormal);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[0]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[1]].Vertex].pos);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[1]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[2]].Vertex].pos);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[2]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[0]].Vertex].pos);
		}
	}*/

	float epsilon = model->getRadius() * 0.001f;
	for (ModelPolygon &p: polygon)
		inside_data->add(p, model, epsilon);
}

void ModelSurface::endInsideTests()
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

bool ModelSurface::insideTest(const vector &p)
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
	for (ModelPolygon &t, Polygon){

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



Array<int> ModelSurface::getBoundaryLoop(int v0)
{
	Array<int> loop;
	int last = v0;
	bool found = true;
	while(found){
		found = false;
		for (ModelEdge &e: edge)
			if (e.ref_count == 1)
				if (e.vertex[0] == last){
					last = e.vertex[1];
					loop.add(last);
					if (last == v0)
						return loop;
					found = true;
					break;
				}
	}
	return loop;
}

int ModelSurface::findEdge(int vertex0, int vertex1)
{
	foreachi(ModelEdge &e, edge, i)
		if (((e.vertex[0] == vertex0) && (e.vertex[1] == vertex1)) || ((e.vertex[1] == vertex0) && (e.vertex[0] == vertex1)))
			return i;
	return -1;
}

Array<Array<int> > ModelSurface::getConnectedComponents()
{
	Array<Array<int> > cc;
	return cc;
}
