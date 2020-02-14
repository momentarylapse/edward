/*
 * ModelMesh.cpp
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#include "ModelMesh.h"
#include "ModelPolygon.h"
#include "ModelSelection.h"
#include "Geometry/Geometry.h"
#include "BspTree.h"
#include "../../lib/file/msg.h"
#include "../../lib/base/set.h"
#include "../../Edward.h"
#include "../../MultiView/MultiView.h"

ModelMesh::ModelMesh(DataModel *m) {
	model = m;
	inside_data = NULL;
}

ModelMesh::~ModelMesh() {
}

void ModelMesh::clear() {
	polygon.clear();
	edge.clear();
	vertex.clear();

	ball.clear();
	cylinder.clear();
}

bool ModelMesh::test_sanity(const string &loc) {

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


bool int_array_has_duplicates(const Array<int> &a) {
	for (int i=0; i<a.num; i++)
		for (int j=i+1; j<a.num; j++)
			if (a[i] == a[j])
				return true;
	return false;
}


void ModelMesh::_addPolygon(const Array<int> &v, int _material, const Array<vector> &sv, int index)
{
	if (int_array_has_duplicates(v))
		throw GeometryException("AddPolygon: duplicate vertices");

	ModelPolygon t;
	t.side.resize(v.num);
	for (int k=0;k<v.num;k++){
		t.side[k].vertex = v[k];
		for (int i=0;i<model->material[_material]->texture_levels.num;i++)
			t.side[k].skin_vertex[i] = sv[i * v.num + k];
	}
	for (int k=0;k<v.num;k++){
		try{
			t.side[k].edge = add_edge_for_new_polygon(t.side[k].vertex, t.side[(k + 1) % v.num].vertex, polygon.num, k);
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

	for (int vv: v)
		vertex[vv].ref_count ++;

	// closed?
//	updateClosed();

	t.is_selected = false;
	t.material = _material;
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

void ModelMesh::_removePolygon(int index)
{
	ModelPolygon &t = polygon[index];

	// unref the vertices
	for (int k=0;k<t.side.num;k++){
		vertex[t.side[k].vertex].ref_count --;
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
		remove_obsolete_edge(o);

/*	if (!TestSanity("rem poly"))
		throw GeometryException("RemoveTriangle: TestSanity failed");*/
}

void ModelMesh::build_topology()
{
	// clear
	edge.clear();
	for (ModelVertex &v: vertex)
		v.ref_count = 0;

	// add all triangles
	foreachi(ModelPolygon &t, polygon, ti){
		// vertices
		for (int k=0;k<t.side.num;k++)
			vertex[t.side[k].vertex].ref_count ++;

		// edges
		for (int k=0;k<t.side.num;k++){
			t.side[k].edge = add_edge_for_new_polygon(t.side[k].vertex, t.side[(k + 1) % t.side.num].vertex, ti, k);
			t.side[k].edge_direction = edge[t.side[k].edge].ref_count - 1;
		}
	}

//	updateClosed();
}


void ModelMesh::remove_obsolete_edge(int index)
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

int ModelMesh::add_edge_for_new_polygon(int a, int b, int tria, int side)
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






void ModelMesh::on_post_action_update() {
	set_show_vertices(vertex);

	update_normals();
	for (ModelPolygon &p: polygon) {
		p.pos = v_0;
		for (int k=0;k<p.side.num;k++)
			p.pos += vertex[p.side[k].vertex].pos;
		p.pos /= p.side.num;
	}
	for (ModelEdge &e: edge)
		e.pos = (vertex[e.vertex[0]].pos + vertex[e.vertex[1]].pos) / 2;
}

void ModelMesh::set_show_vertices(Array<ModelVertex> &vert) {
	show_vertices.set_ref(vert);
}



void report_error(const string &msg)
{
	msg_error(msg);
	if (ed)
		ed->set_message(msg);
}



#if 0
void ModelMesh::importFromTriangleSkin(int index) {
	vertex.clear();
	polygon.clear();
	edge.clear();

	ModelSkin &s = skin[index];
	begin_action_group("ImportFromTriangleSkin");
	foreachi(ModelVertex &v, s.vertex, i){
		addVertex(v.pos);
		vertex[i].bone_index = v.bone_index;
	}
	for (int i=0;i<material.num;i++){
		for (ModelTriangle &t: s.sub[i].triangle){
			if ((t.vertex[0] == t.vertex[1]) || (t.vertex[1] == t.vertex[2]) || (t.vertex[2] == t.vertex[0]))
				continue;
			Array<int> v;
			for (int k=0;k<3;k++)
				v.add(t.vertex[k]);
			Array<vector> sv;
			for (int tl=0;tl<material[i]->texture_levels.num;tl++)
				for (int k=0;k<3;k++)
					sv.add(t.skin_vertex[tl][k]);
			try{
				addPolygonWithSkin(v, sv, i);
			}catch(GeometryException &e){
				msg_error("polygon..." + e.message);
				msg_write("trying to copy vertices...");

				// copy all vertices m(-_-)m
				int nv0 = vertex.num;
				for (int k=0; k<3; k++){
					addVertex(vertex[v[k]].pos);
					vertex.back().bone_index = vertex[v[k]].bone_index;
					v[k] = nv0 + k;
				}
				addPolygonWithSkin(v, sv, i);
			}
		}
	}


	ModelSkin &ps = skin[0];
	foreachi(ModelVertex &v, ps.vertex, i){
		addVertex(v.pos);
		vertex[i].bone_index = v.bone_index;
	}
	for (ModelPolyhedron &p: polyhedron){
		msg_write("----");
		int nv0 = vertex.num;
		Array<int> vv;
		for (int i=0;i<p.NumFaces;i++){
			Array<int> v;
			for (int j=0;j<p.Face[i].NumVertices;j++){
				int vj = p.Face[i].Index[j];
				bool existing = false;
				for (int k=0;k<vv.num;k++)
					if (vv[k] == vj){
						existing = true;
						v.add(nv0 + k);
						break;
					}
				if (!existing){
					v.add(vertex.num);
					vv.add(vj);
					addVertex(ps.vertex[vj].pos);
					vertex.back().bone_index = ps.vertex[vj].bone_index;
				}
			}
			msg_write(ia2s(v));
			try{
				addPolygon(v, 0);
			}catch(GeometryException &e){
				msg_error(e.message);
			}
		}
		//surface.back().is_physical = true;
		//surface.back().is_visible = false;
	}
	polyhedron.clear();

	clearSelection();
	end_action_group();
	action_manager->reset();
}
#endif

void ModelMesh::exportTo_triangle_skin(ModelSkin &sk) {
	sk.vertex = vertex;
	sk.sub.clear();
	sk.sub.resize(model->material.num);
	for (auto &t: polygon) {
		if (t.triangulation_dirty)
			t.updateTriangulation(vertex);
		for (int i=0;i<t.side.num-2;i++) {
			ModelTriangle tt;
			for (int k=0;k<3;k++) {
				tt.vertex[k] = t.side[t.side[i].triangulation[k]].vertex;
				tt.normal[k] = t.side[t.side[i].triangulation[k]].normal;
				for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
					tt.skin_vertex[l][k] = t.side[t.side[i].triangulation[k]].skin_vertex[l];
			}
			sk.sub[t.material].triangle.add(tt);
		}
	}
	foreachi(auto *m, model->material, i)
		sk.sub[i].num_textures = m->texture_levels.num;
}


void ModelMesh::get_bounding_box(vector &min, vector &max, bool dont_reset) {
	if (!dont_reset)
		min = max = v_0;

	for (auto &v: vertex){
		min._min(v.pos);
		max._max(v.pos);
	}

	for (auto &b: ball){
		min._min(vertex[b.index].pos - vector(1,1,1) * b.radius);
		max._max(vertex[b.index].pos + vector(1,1,1) * b.radius);
	}
}

void ModelMesh::set_normals_dirty_by_vertices(const Array<int> &index)
{
	Set<int> sindex;
	for (int i=0; i<index.num; i++)
		sindex.add(index[i]);

	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			if (!t.normal_dirty)
				if (sindex.contains(t.side[k].vertex)){
					t.normal_dirty = true;
					break;
				}
}

void ModelMesh::set_all_normals_dirty() {
	for (ModelPolygon &t: polygon)
		t.normal_dirty = true;
}

struct PolySideData {
	int poly;
	int side;
};


inline int find_other_tria_from_edge(ModelMesh *m, int e, int t) {
	if (m->edge[e].polygon[0] == t)
		return m->edge[e].polygon[1];
	return m->edge[e].polygon[0];
}

// return: closed circle... don't run again to the left
inline bool find_tria_top(ModelMesh *m, const Array<PolySideData> &pd, Set<int> &used, bool to_the_right) {
	int t0 = 0;
	while(true){
		int side = pd[t0].side;
		if (!to_the_right){
			int ns = m->polygon[pd[t0].poly].side.num;
			side = (side + ns - 1) % ns;
		}
		int e = m->polygon[pd[t0].poly].side[side].edge;
		if (!m->edge[e].is_round)
			return false;
		int tt = find_other_tria_from_edge(m, e, pd[t0].poly);
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


void ModelMesh::update_normals()
{
	Set<int> ee, vert;

	// "flat" triangle normals
	for (ModelPolygon &t: polygon)
		if (t.normal_dirty){
			t.normal_dirty = false;

			t.temp_normal = t.getNormal(vertex);

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

		ModelVertex &v1 = vertex[e.vertex[0]];
		ModelVertex &v2 = vertex[e.vertex[1]];

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
		if (vertex[ip].normal_mode == NORMAL_MODE_HARD)
			continue;

		Array<PolySideData> &pd = poly_side[nn];

		// smooth vertex
		if (vertex[ip].normal_mode == NORMAL_MODE_SMOOTH){

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



void ModelMesh::clear_selection()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelPolygon &t: polygon)
		t.is_selected = false;
	for (ModelEdge &e: edge)
		e.is_selected = false;
	model->notify(model->MESSAGE_SELECTION);
}

void ModelMesh::selection_from_polygons() {
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelEdge &e: edge)
		e.is_selected = false;
	for (ModelPolygon &t: polygon)
		if (t.is_selected)
			for (int k=0;k<t.side.num;k++){
				vertex[t.side[k].vertex].is_selected = true;
				edge[t.side[k].edge].is_selected = true;
			}
	model->notify(model->MESSAGE_SELECTION);
}

void ModelMesh::selection_from_edges() {
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelEdge &e: edge)
		if (e.is_selected)
			for (int k=0;k<2;k++)
				vertex[e.vertex[k]].is_selected = true;
	for (ModelPolygon &p: polygon){
		p.is_selected = true;
		for (int k=0;k<p.side.num;k++)
			p.is_selected &= edge[p.side[k].edge].is_selected;
	}
	model->notify(model->MESSAGE_SELECTION);
}

void ModelMesh::selection_from_vertices() {
	for (ModelEdge &e: edge) {
		e.is_selected = (vertex[e.vertex[0]].is_selected and vertex[e.vertex[1]].is_selected);
		e.view_stage = min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage);
	}
	for (ModelPolygon &t: polygon) {
		t.is_selected = true;
		t.view_stage = vertex[t.side[0].vertex].view_stage;
		for (int k=0;k<t.side.num;k++) {
			t.is_selected &= vertex[t.side[k].vertex].is_selected;
			t.view_stage = min(t.view_stage, vertex[t.side[k].vertex].view_stage);
		}
	}
	model->notify(model->MESSAGE_SELECTION);
}




struct MeshInsideTestData : BspTree{};

void ModelMesh::begin_inside_tests()
{
#if 0
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

	float epsilon = getRadius() * 0.001f;
	for (ModelPolygon &p: polygon)
		inside_data->add(p, this, epsilon);
#endif
}

bool ModelMesh::inside_test(const vector &p)
{
	if (!inside_data)
		return false;
	return inside_data->inside(p);

}

void ModelMesh::end_inside_tests()
{
//	if (inside_data)
	//	delete(inside_data);
}



ModelSelection ModelMesh::get_selection() const {
	ModelSelection s;
	foreachi(ModelVertex &v, vertex, i)
		if (v.is_selected)
			s.vertex.add(i);
	foreachi(ModelPolygon &t, polygon, j)
		if (t.is_selected)
			s.polygon.add(j);
	foreachi(ModelEdge &e, edge, j)
		if (e.is_selected)
			s.edge.add(j);
	return s;
}

void ModelMesh::set_selection(const ModelSelection &s) {
	clear_selection();
	for (int v: s.vertex)
		vertex[v].is_selected = true;
	for (int p: s.polygon)
		polygon[p].is_selected = true;
	for (int e: s.edge)
		edge[e].is_selected = true;
	model->notify(model->MESSAGE_SELECTION);
}


Array<int> ModelMesh::get_boundary_loop(int v0)
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

void ModelMesh::add_vertex(const vector &pos, int bone, int normal_mode, int index) {

	// new vertex
	ModelVertex vv;
	vv.pos = pos;
	vv.normal_mode = normal_mode;
	vv.bone_index = bone;
	vv.view_stage = ed->multi_view_3d->view_stage;
	vv.ref_count = 0;
	if (index >= 0) {
		vertex.insert(vv, index);
		_shift_vertex_links(index, 1);

		// correct animations
	} else {
		vertex.add(vv);
	}

}

void ModelMesh::remove_lonely_vertex(int index) {
	// not done:
	// move data
	// fx

	_shift_vertex_links(index, -1);


	// erase
	vertex.erase(index);
}

void ModelMesh::_shift_vertex_links(int offset, int delta) {

	// correct references
	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].vertex >= offset)
				t.side[k].vertex += delta;
	for (ModelEdge &e: edge)
		for (int k=0;k<2;k++)
			if (e.vertex[k] >= offset)
				e.vertex[k] += delta;
}
