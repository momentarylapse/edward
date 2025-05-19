//
// Created by Michael Ankele on 2025-05-19.
//

#include "MeshEdit.h"
#include <lib/base/sort.h>
#include <lib/base/iter.h>


void MeshEdit::delete_vertex(int index) {
	_del_vertices.add(index);
}

void MeshEdit::delete_polygon(int index) {
	_del_polygons.add(index);
}

int MeshEdit::add_vertex(const MeshVertex& v) {
	_new_vertices.add(v);
	return -_new_vertices.num; // starts at -1!
}

void MeshEdit::add_polygon(const Polygon& p) {
	_new_polygons.add(p);
}



MeshEdit MeshEdit::apply_inplace(PolygonMesh& mesh) const {
	MeshEdit inv;

	// delete vertices
	for (int i: _del_vertices)
		inv.add_vertex(mesh.vertices[i]);
	for (int i: base::reverse(_del_vertices))
		mesh.vertices.erase(i);

	// add vertices
	for (const auto& [i, v]: enumerate(_new_vertices)) {
		inv.delete_vertex(mesh.vertices.num);
		mesh.vertices.add(v);
	}

	// add polygons
	for (const auto& p: _new_polygons) {
		inv.delete_polygon(mesh.polygons.num - _del_polygons.num);
		mesh.polygons.add(p);
		mesh.polygons.back().normal_dirty = true;
	}

	auto remap = [this, &mesh] (int index) {
		if (index < 0)
			// newly added
				return mesh.vertices.num - _new_vertices.num - (index + 1);

		for (int i=_del_vertices.num-1; i>=0; i--) {
			if (index == _del_vertices[i])
				index = -1 - i;
			if (index > _del_vertices[i])
				index --;
		}
		return index;
	};

	// remap
	for (auto& p: mesh.polygons)
		for (auto& s: p.side)
			s.vertex = remap(s.vertex);
	for (auto& s: mesh.spheres)
		s.index = remap(s.index);
	for (auto& c: mesh.cylinders)
		for (int k=0; k<2; k++)
			c.index[k] = remap(c.index[k]);

	// delete polygons
	for (int i: base::reverse(_del_polygons)) {
		auto p = mesh.polygons[i];
		//for (auto& s: p.side)
		//	s.vertex = remap(...);
		inv.add_polygon(p);
		mesh.polygons.erase(i);
	}

	return inv;
}

PolygonMesh MeshEdit::apply(const PolygonMesh& mesh, MeshEdit* inv) const {
	PolygonMesh mesh2 = mesh;
	if (inv)
		*inv = apply_inplace(mesh2);
	else
		apply_inplace(mesh2);
	return mesh2;
}

