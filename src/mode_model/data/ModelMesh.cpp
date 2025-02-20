/*
 * ModelMesh.cpp
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */
#include "ModelMesh.h"
//#include "ModelSelection.h"
#include "DataModel.h"
//#include "BspTree.h"
#include <lib/os/msg.h>
#include <lib/base/set.h>
#include <lib/math/Box.h>
//#include "../../EdwardWindow.h"
#include <y/world/components/Animator.h>
#include <y/world/Model.h>




ModelMesh::ModelMesh(DataModel *m) {
	model = m;
	inside_data = nullptr;
}

ModelMesh::~ModelMesh() = default;

void ModelMesh::clear() {
	PolygonMesh::clear();

	ball.clear();
	cylinder.clear();
}

bool ModelMesh::test_sanity(const string &loc) {

	for (auto &t: polygons)
		for (int k=0;k<t.side.num;k++)
			for (int kk=k+1;kk<t.side.num;kk++)
				if (t.side[k].vertex == t.side[kk].vertex){
					msg_error(loc + ": surf broken!   identical vertices in poly");
					return false;
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


void ModelMesh::_add_polygon(const Array<int> &v, int _material, const Array<vec3> &sv, int index) {
	if (int_array_has_duplicates(v))
		throw GeometryException("AddPolygon: duplicate vertices");

	Polygon t;
	t.side.resize(v.num);
	for (int k=0;k<v.num;k++) {
		t.side[k].vertex = v[k];
		for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
			t.side[k].skin_vertex[i] = sv[i * v.num + k];
	}

	for (int vv: v)
		vertices[vv].ref_count ++;

	// closed?
//	updateClosed();

	t.material = _material;
	t.normal_dirty = true;
	t.triangulation_dirty = true;
	t.smooth_group = -1;
	if (index >= 0) {
		polygons.insert(t, index);
	} else {
		polygons.add(t);
	}
}

void ModelMesh::_remove_polygon(int index)
{
	auto &t = polygons[index];

	// unref the vertices
	for (int k=0;k<t.side.num;k++){
		vertices[t.side[k].vertex].ref_count --;
	}


	polygons.erase(index);

	//TestSanity("rem poly 0");

/*	if (!TestSanity("rem poly"))
		throw GeometryException("RemoveTriangle: TestSanity failed");*/
}

void ModelMesh::build_topology()
{
	// clear
	for (auto &v: vertices)
		v.ref_count = 0;

	// add all triangles
	foreachi(Polygon &t, polygons, ti){
		// vertices
		for (int k=0;k<t.side.num;k++)
			vertices[t.side[k].vertex].ref_count ++;
	}
}



void ModelMesh::on_post_action_update() {
	set_show_vertices(vertices);

	update_normals();
	for (auto &p: polygons) {
		p.pos = v_0;
		for (int k=0;k<p.side.num;k++)
			p.pos += vertices[p.side[k].vertex].pos;
		p.pos /= p.side.num;
	}
}

void ModelMesh::set_show_vertices(Array<MeshVertex> &vert) {
	show_vertices.set_ref(vert);
}



/*void report_error(const string &msg)
{
	msg_error(msg);
	if (ed)
		session->set_message(msg);
}*/



#if 0
void ModelMesh::importFromTriangleSkin(int index) {
	vertex.clear();
	polygon.clear();
	edge.clear();

	ModelTriangleMesh &s = skin[index];
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
			Array<vec3> sv;
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


	ModelTriangleMesh &ps = skin[0];
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

void ModelMesh::export_to_triangle_mesh(ModelTriangleMesh &sk) {
	sk.vertices = vertices;
	sk.sub.clear();
	sk.sub.resize(model->material.num);
	for (auto &t: polygons) {
		if (t.triangulation_dirty)
			t.update_triangulation(vertices);
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

Box ModelMesh::get_bounding_box() {
	Box box = {v_0, v_0};

	for (const auto &v: vertices)
		box = box or Box{v.pos, v.pos};

	for (const auto &b: ball)
		box = box or Box{
			vertices[b.index].pos - vec3(1,1,1) * b.radius,
			vertices[b.index].pos + vec3(1,1,1) * b.radius};
	return box;
}

void ModelMesh::set_normals_dirty_by_vertices(const Array<int> &index)
{
	base::set<int> sindex;
	for (int i=0; i<index.num; i++)
		sindex.add(index[i]);

	for (auto &t: polygons)
		for (int k=0;k<t.side.num;k++)
			if (!t.normal_dirty)
				if (sindex.contains(t.side[k].vertex)){
					t.normal_dirty = true;
					break;
				}
}

void ModelMesh::set_all_normals_dirty() {
	for (auto &t: polygons)
		t.normal_dirty = true;
}


void ModelMesh::update_normals() {
	if (this == model->phys_mesh) {
		for (auto &v: vertices)
			v.normal_mode = NORMAL_MODE_HARD;
	}

#define NEW_NORMALS 1

#if NEW_NORMALS


	// "flat" triangle normals
	for (auto &t: polygons)
		if (t.normal_dirty) {
			t.normal_dirty = false;
			t.temp_normal = t.get_normal(vertices);
			for (int k=0; k<t.side.num; k++)
				t.side[k].normal = t.temp_normal;
		}

	Array<int> cur_polys;
	Array<int> cur_faces;
	Array<int> cur_groups;
	base::set<int> cur_groups_done;
	for (int v=0; v<vertices.num; v++) {
		cur_polys.clear();
		cur_faces.clear();
		cur_groups.clear();
		cur_groups_done.clear();
		foreachi(auto &p, polygons, i) {
			if (p.smooth_group < 0)
				continue;
			for (int k=0; k<p.side.num; k++) {
				if (p.side[k].vertex == v) {
					cur_polys.add(i);
					cur_faces.add(k);
					cur_groups.add(p.smooth_group);
				}
			}
		}
		for (int i=0; i<cur_groups.num; i++) {
			int g = cur_groups[i];
			if (cur_groups_done.contains(g))
				continue;
			vec3 n = v_0;
			for (int k=i; k<cur_groups.num; k++)
				if (cur_groups[k] == g)
					n += polygons[cur_polys[k]].temp_normal;
			n.normalize();
			for (int k=i; k<cur_groups.num; k++)
				if (cur_groups[k] == g)
					polygons[cur_polys[k]].side[cur_faces[k]].normal = n;
			cur_groups_done.add(g);
		}
	}

#else


	base::set<int> ee, vert;

	// "flat" triangle normals
	for (ModelPolygon &t: polygon)
		if (t.normal_dirty) {
			t.normal_dirty = false;
			t.temp_normal = t.get_normal(vertex);

			for (int k=0;k<t.side.num;k++) {
				t.side[k].normal = t.temp_normal;
				int e = t.side[k].edge;
				if (edge[e].ref_count == 2)
					ee.add(e);
			}
		}

	// round edges?
	for (int ip: ee) {
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
			vec3 n = v_0;
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
			base::set<int> used;
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
			vec3 n = v_0;
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
#endif
}



#if 0
void ModelMesh::clear_selection()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelPolygon &t: polygon)
		t.is_selected = false;
	for (ModelEdge &e: edge)
		e.is_selected = false;
	model->out_selection();
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
	model->out_selection();
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
	model->out_selection();
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
	for (auto& b: ball) {
		b.view_stage = vertex[b.index].view_stage;
		b.is_selected = vertex[b.index].is_selected;
	}
	model->out_selection();
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

bool ModelMesh::inside_test(const vec3 &p)
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
			s.set(e);
	foreachi (auto &b, ball, i)
		if (b.is_selected)
			s.ball.add(i);
	foreachi (auto &c, cylinder, i)
		if (c.is_selected)
			s.cylinder.add(i);
	return s;
}

void ModelMesh::set_selection(const ModelSelection &s) {
	clear_selection();
	for (int v: s.vertex)
		vertex[v].is_selected = true;
	for (int p: s.polygon)
		polygon[p].is_selected = true;
	for (auto &e: edge)
		e.is_selected = s.has(e);
	model->out_selection();
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
#endif

void ModelMesh::add_vertex(const vec3 &pos, const ivec4 &bone, const vec4 &bone_weight, int normal_mode, int index) {

	// new vertex
	MeshVertex vv;
	vv.pos = pos;
	vv.normal_mode = normal_mode;
	vv.bone_index = bone;
	vv.bone_weight = bone_weight;
	vv.ref_count = 0;
	if (index >= 0) {
		vertices.insert(vv, index);
		_shift_vertex_links(index, 1);

		// correct animations
	} else {
		vertices.add(vv);
	}

}

void ModelMesh::_add_vertices(const Array<MeshVertex> &v) {
	vertices.append(v);
	_post_vertex_number_change_update();
}

void ModelMesh::_post_vertex_number_change_update() {
	// resize animations
	for (ModelMove &move: model->move) {
		if (move.type == AnimationType::VERTEX) {
			for (ModelFrame &f: move.frame)
				f.vertex_dpos.resize(vertices.num);
		}
	}
}

void ModelMesh::remove_lonely_vertex(int index) {
	// not done:
	// move data
	// fx

	_shift_vertex_links(index, -1);


	// erase
	vertices.erase(index);
}

void ModelMesh::_shift_vertex_links(int offset, int delta) {

	// correct references
	for (auto &t: polygons)
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].vertex >= offset)
				t.side[k].vertex += delta;
	for (ModelBall& b: ball)
		if (b.index >= offset)
			b.index += delta;
}

#if 0
// TODO
float ModelBall::hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) {
	return MultiView::SingleData::hover_distance(win, m, tp, z);
}

bool ModelBall::in_rect(MultiView::Window *win, const rect &r) {
	return MultiView::SingleData::in_rect(win, r);
}

bool ModelBall::overlap_rect(MultiView::Window *win, const rect &r) {
	return MultiView::SingleData::in_rect(win, r);
}

float ModelCylinder::hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) {
	return MultiView::SingleData::hover_distance(win, m, tp, z);
}

bool ModelCylinder::in_rect(MultiView::Window *win, const rect &r) {
	return MultiView::SingleData::in_rect(win, r);
}

bool ModelCylinder::overlap_rect(MultiView::Window *win, const rect &r) {
	return MultiView::SingleData::in_rect(win, r);
}


Geometry ModelMesh::copy_geometry() {
	Geometry geo;

	// copy vertices
	Array<int> vert;
	foreachi(ModelVertex &v, vertex, vi)
		if (v.is_selected) {
			geo.vertex.add(v);
			vert.add(vi);
		}

	// copy triangles
	for (ModelPolygon &t: polygon)
		if (t.is_selected) {
			ModelPolygon tt = t;
			for (int k=0;k<t.side.num;k++)
				foreachi(int v, vert, vi)
					if (v == t.side[k].vertex)
						tt.side[k].vertex = vi;
			geo.polygon.add(tt);
		}
	return geo;
}
#endif
