//
// Created by michi on 19.05.25.
//

#ifndef NDEBUG

#include "MeshTest.h"
#include <data/mesh/PolygonMesh.h>
#include <data/mesh/GeometryCube.h>
#include <lib/os/msg.h>

#include "lib/base/iter.h"
#include "lib/math/Box.h"
#include "lib/math/random.h"

namespace unittest {

static constexpr bool verbose = false;

void show_mesh(const PolygonMesh& mesh) {
	msg_write("-----");
	for (const auto& v: mesh.vertices)
		msg_write(str(v.pos));
	for (const auto& p: mesh.polygons)
		msg_write(str(p.get_vertices()));
}

void show_mesh_diff(const MeshEdit& edit) {
	msg_write("-  " + str(edit._del_vertices));
	msg_write("+  " + str(edit._new_vertices.num));
	msg_write("-- " + str(edit._del_polygons));
	for (const auto& p: edit._new_polygons)
		msg_write("++ " + str(p.get_vertices()));
}

void assert_equal(const PolygonMesh &a, const PolygonMesh &b, float epsilon = 0.001f) {
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
	list.add({"diff_invertible", MeshTest::test_diff_invertible});
	return list;
}

void MeshTest::test_diff_invertible() {
	const PolygonMesh mesh0 = GeometryCube::create(Box::ID_SYM, {1,1,1});
	if constexpr (verbose)
		show_mesh(mesh0);

	for (int i=0; i<1000; i++) {
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
		check_mesh_health(mesh);
		if constexpr (verbose)
			show_mesh(mesh);

		assert_equal(mesh, mesh0);
	}
}

}

#endif
