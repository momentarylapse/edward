//
// Created by michi on 19.05.25.
//

#include <Session.h>
#include <lib/base/sort.h>
#ifndef NDEBUG

#include "MeshTest.h"
#include <data/mesh/PolygonMesh.h>
#include <data/mesh/MeshEdit.h>
#include <data/mesh/GeometryCube.h>
#include <data/mesh/GeometryPlane.h>
#include <mode_model/mesh/processing/MeshExtrudePolygons.h>
#include <lib/os/msg.h>
#include <lib/base/iter.h>
#include <lib/math/Box.h>
#include <lib/math/random.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>

namespace unittest {

static constexpr bool verbose = false;

void show_mesh(const PolygonMesh& mesh) {
	msg_write("-----");
	for (const auto& v: mesh.vertices)
		msg_write(str(v.pos));
	for (const auto& p: mesh.polygons)
		msg_write(str(p.get_vertices()));
}

void show_mesh_diff(const MeshEdit& edit);

void assert_equal(const PolygonMesh &a, const PolygonMesh &b, float epsilon = 0.001f) {
	if (a.vertices.num != b.vertices.num)
		throw Failure(format("Mesh #vertices: %d != %d", a.vertices.num, b.vertices.num));
	if (a.polygons.num != b.polygons.num)
		throw Failure(format("Mesh #polygons: %d != %d", a.polygons.num, b.polygons.num));

	// check vertices
	for (int i=0; i<a.vertices.num; i++)
		assert_equal(a.vertices[i].pos, b.vertices[i].pos);

	for (int i=0; i<a.polygons.num; i++)
		assert_equal(a.polygons[i].get_vertices(), b.polygons[i].get_vertices());
}

void assert_equal_up_to_permutation(const PolygonMesh &a, const PolygonMesh &b, float epsilon = 0.001f) {
	if (a.vertices.num != b.vertices.num)
		throw Failure(format("Mesh #vertices: %d != %d", a.vertices.num, b.vertices.num));
	if (a.polygons.num != b.polygons.num)
		throw Failure(format("Mesh #polygons: %d != %d", a.polygons.num, b.polygons.num));

	// find vertex mapping
	base::map<int,int> vmap; // a -> b
	for (int i=0; i<a.vertices.num; i++)
		for (int j=0; j<b.vertices.num; j++)
			if ((a.vertices[i].pos - b.vertices[j].pos).length() < epsilon)
				vmap.set(i, j);
	if constexpr (verbose)
		msg_write("vmap " + str(vmap));
	if (vmap.num != a.vertices.num)
		throw Failure("Mesh vmap...");

	// check vertices
	for (int i=0; i<a.vertices.num; i++)
		assert_equal(a.vertices[i].pos, b.vertices[vmap[i]].pos);

	// find polygon map
	base::map<int,int> pmap;
	for (const auto& [ia, pa]: enumerate(a.polygons))
		for (const auto& [ib, pb]: enumerate(b.polygons)) {
			if (pa.side.num != pb.side.num)
				continue;

			bool match = true;
			for (int k=0; k<pa.side.num; k++)
				if (vmap[pa.side[k].vertex] != pb.side[k].vertex)
					match = false;
			if (match)
				pmap.set(ia, ib);
		}
	if constexpr (verbose)
		msg_write("pmap " + str(pmap));
}

void check_mesh_health(const PolygonMesh& mesh) {
	for (const auto& p: mesh.polygons)
		for (const auto& s: p.side)
			if (s.vertex < 0 or s.vertex >= mesh.vertices.num)
				throw Failure(format("Mesh  illegal vertex ref: %d  (n=%d)", s.vertex, mesh.vertices.num));
}

MeshEdit random_mesh_edit(const PolygonMesh &mesh, int seed) {
	Random r;
	r.seed(str(seed));

	MeshEdit edit;

	if (mesh.vertices.num > 0) {
		int v_del = r._int(mesh.vertices.num);
		edit.delete_vertex(v_del);
		for (const auto& [i, p]: enumerate(mesh.polygons)) {
			for (const auto& s: p.side)
				if (s.vertex == v_del)
					edit.delete_polygon(i);
		}
	}

	Array<int> new_vertices;
	for (int i=0; i<2; i++)
		new_vertices.add(edit.add_vertex(MeshVertex{r.in_ball(20)}));

	for (int i=0; i<2; i++) {
		Polygon polygon;
		int n = 3 + r._int(2);
		polygon.side.resize(n);
		for (int j=0; j<n; j++) {
			do {
				polygon.side[j].vertex = r._int(mesh.vertices.num + new_vertices.num) - new_vertices.num;
			} while (edit._del_vertices.contains(polygon.side[j].vertex));
		}
		edit.add_polygon(polygon);
	}

	return edit;
}

MeshTest::MeshTest() : UnitTest("mesh") {
}

Array<UnitTest::Test> MeshTest::tests() {
	Array<Test> list;
	list.add({"diff_basic_vertices", MeshTest::test_diff_basic_vertices});
	list.add({"diff_invertible", MeshTest::test_diff_invertible});
	list.add({"diff_iterated", MeshTest::test_diff_iterated});
	list.add({"extrude", MeshTest::test_extrude});
	list.add({"extrude_undo_redo", MeshTest::test_extrude_undo_redo});
	return list;
}

void MeshTest::test_diff_basic_vertices() {
	PolygonMesh mesh0;
	for (int i=0; i<5; i++)
		mesh0.add_vertex(vec3((float)i, 0, 0));
	mesh0.add_polygon_auto_texture({0,2,4});
	//show_mesh(mesh0);

	MeshEdit ed;
	ed.delete_vertex(1);
	ed.delete_vertex(3);
	ed.add_vertex(MeshVertex(vec3(991,0,0)), 0);
	ed.add_vertex(MeshVertex(vec3(992,0,0)), 3);
	ed.add_vertex(MeshVertex(vec3(993,0,0)), 3);
	ed.add_vertex(MeshVertex(vec3(994,0,0)));
	//show_mesh_diff(ed);

	PolygonMesh mesh1_expected;
	mesh1_expected.add_vertex(vec3(991,0,0));
	mesh1_expected.add_vertex(vec3(0,0,0));
	mesh1_expected.add_vertex(vec3(2,0,0));
	mesh1_expected.add_vertex(vec3(992,0,0));
	mesh1_expected.add_vertex(vec3(993,0,0));
	mesh1_expected.add_vertex(vec3(4,0,0));
	mesh1_expected.add_vertex(vec3(994,0,0));
	mesh1_expected.add_polygon_auto_texture({1,2,5});

	MeshEdit inv;
	auto mesh1 = ed.apply(mesh0, &inv);
	//show_mesh(mesh1);
	assert_equal(mesh1, mesh1_expected);
	//show_mesh_diff(inv);

	MeshEdit invinv;
	auto mesh0b = inv.apply(mesh1, &invinv);
	//show_mesh(mesh0b);
	assert_equal(mesh0b, mesh0);
	//show_mesh_diff(invinv);

	auto mesh1b = invinv.apply(mesh0b);
	//show_mesh(mesh1b);
	assert_equal(mesh1b, mesh1);
}


void MeshTest::test_diff_invertible() {
	//const PolygonMesh mesh0 = GeometryCube::create(Box::ID_SYM, {1,1,1});
	const PolygonMesh mesh0 = GeometryPlane::create(rect::ID_SYM, {1,1});
	if constexpr (verbose)
		show_mesh(mesh0);

	for (int i=0; i<100; i++) {
		auto ed = random_mesh_edit(mesh0, i);
		if constexpr (verbose)
			show_mesh_diff(ed);

		auto mesh = mesh0;
		auto inv = mesh.edit_inplace(ed);
		if constexpr (verbose)
			show_mesh(mesh);
		check_mesh_health(mesh);

		if constexpr (verbose)
			show_mesh_diff(inv);
		mesh.edit_inplace(inv);
		if constexpr (verbose)
			show_mesh(mesh);
		check_mesh_health(mesh);

		assert_equal(mesh, mesh0);
	}
}

void MeshTest::test_diff_iterated() {
	const PolygonMesh mesh0 = GeometryCube::create(Box::ID_SYM, {1,1,1});

	for (int i=0; i<100; i++) {
		//msg_write("====");
		//msg_write(i);
		Array<MeshEdit> inv;
		Array<PolygonMesh> meshes;

		// forward
		auto mesh = mesh0;
		//show_mesh(mesh);
		for (int k=0; k<15; k++) {
			//msg_write(format("+++++++ %d" ,k));
			meshes.add(mesh);

			const auto ed = random_mesh_edit(mesh, 42+i + k);
			//show_mesh_diff(ed);
			inv.add(ed.apply_inplace(mesh));
			//show_mesh(mesh);
			check_mesh_health(mesh);
		}

		// backward
		for (int k=meshes.num-1; k>=0; k--) {
			//msg_write(format("--------- %d" ,k));
			inv[k].apply_inplace(mesh);
			//show_mesh_diff(inv[k]);
			//show_mesh(mesh);
			check_mesh_health(mesh);
			assert_equal(mesh, meshes[k]);
		}
	}
}

Data::Selection mesh_select_random_polygons(const PolygonMesh& mesh, int seed) {
	Data::Selection sel;
	sel.set(MultiViewType::MODEL_POLYGON, {});

	Random r;
	r.seed(str(seed));
	for (int i=0; i<4; i++)
		sel[MultiViewType::MODEL_POLYGON].add(r._int(mesh.polygons.num));
	return sel;
}

void MeshTest::test_extrude() {
	const PolygonMesh mesh0 = GeometryCube::create(Box::ID_SYM, {1,1,1});

	Array<MeshEdit> edits;
	Array<MeshEdit> inv;
	Array<MeshEdit> invinv;

	PolygonMesh mesh = mesh0;
	for (int i=0; i<15; i++) {
		auto sel = mesh_select_random_polygons(mesh, i);
		auto ed = mesh_prepare_extrude_polygons(mesh, sel, 0.1f, false);
		edits.add(ed);
		inv.add(ed.apply_inplace(mesh));
		check_mesh_health(mesh);
	}

	for (const auto& i: base::reverse(inv)) {
		invinv.add(i.apply_inplace(mesh));
		check_mesh_health(mesh);
	}

	assert_equal(mesh, mesh0);

	// FIXME inversion...
	/*show_mesh_diff(edits[0]);
	show_mesh_diff(inv.back());
	show_mesh_diff(invinv.back());*/

	for (const auto& ii: base::reverse(invinv)) {
		ii.apply_inplace(mesh);
		check_mesh_health(mesh);
	}
}

void MeshTest::test_extrude_undo_redo() {
	const PolygonMesh mesh0 = GeometryCube::create(Box::ID_SYM, {1,1,1});

	PolygonMesh mesh = mesh0;
	Data::Selection sel;
	sel.set(MultiViewType::MODEL_POLYGON, {0});
	auto ed = mesh_prepare_extrude_polygons(mesh, sel, 0.1f, false);

	Array<PolygonMesh> meshes;

	for (int i=0; i<3; i++) {
		ed = ed.apply_inplace(mesh);
		check_mesh_health(mesh);
		meshes.add(mesh);
	}

	assert_equal(meshes[1], mesh0);
	assert_equal(meshes[2], meshes[0]);
}


}

#endif
