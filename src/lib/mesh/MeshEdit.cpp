//
// Created by Michael Ankele on 2025-05-19.
//

#include "MeshEdit.h"

#include <lib/base/algo.h>
#include <lib/base/sort.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>



void show_mesh_diff(const MeshEdit& edit) {
	msg_write("-  " + str(edit._del_vertices));
	Array<int> at;
	for (const auto& nv: edit._new_vertices)
		at.add(nv.at);
	msg_write("+  " + str(at));
	msg_write("-- " + str(edit._del_polygons));
	for (const auto& p: edit._new_polygons)
		msg_write(format("++ %s @%d",  str(p.p.get_vertices()), p.at));
}

void MeshEdit::delete_vertex(int index) {
	_del_vertices.add(index);
}

void MeshEdit::delete_polygon(int index) {
	_del_polygons.add(index);
}

int MeshEdit::add_vertex(const MeshVertex& v, int at_index) {
	int id = -_new_vertices.num - 1; // starts at -1!
	_new_vertices.add({v, at_index, id});
	return id;
}

void MeshEdit::add_polygon(const Polygon& p, int at_index) {
	_new_polygons.add({p, at_index});
}

MeshEdit MeshEdit::apply_inplace(PolygonMesh& mesh) const {
	MeshEdit inv;
	PolygonMesh out;
	base::map<int,int> del_map;
	base::map<int,int> add_map;

	// hmm, we still have don't perfectly invert MeshEdit... but it seems to perfectly apply/invert on Meshes
	// (appended vertices at -1 => inv(inv) => insert at Mesh.vertices.num)

	// add/delete vertices
	{
		out.vertices.resize(mesh.vertices.num - _del_vertices.num + _new_vertices.num);
		int i_out = 0;
		// TODO optimize...
		for (int i=0; i<mesh.vertices.num; i++) { // in
			// insert new
			for (const auto& nv: _new_vertices)
				if (nv.at == i) {
					add_map.set(nv.id, i_out);
					inv.delete_vertex(i_out);
					out.vertices[i_out ++] = nv.v;
				}

			if (_del_vertices.contains(i)) {
				// delete
				int id = inv.add_vertex(mesh.vertices[i], i_out);
				del_map.set(i, id);
			} else {
				// keep
				out.vertices[i_out ++] = mesh.vertices[i];
			}
		}
		// append new
		for (const auto& nv: _new_vertices)
			if (nv.at == -1 or nv.at == mesh.vertices.num) {
				add_map.set(nv.id, i_out);
				inv.delete_vertex(i_out);
				out.vertices[i_out ++] = nv.v;
			}
	}

	// remap indices
	auto _remap = [this, &add_map, &del_map] (int index) {
		if (index < 0)
			return add_map[index];
		if (_del_vertices.contains(index))
			return del_map[index];
		int n_del_before = base::count_if(_del_vertices, [index] (int d) {
			return d <= index;
		});
		int n_insert_before = base::count_if(_new_vertices, [index] (const NewVertex& nv) {
			return nv.at >= 0 and nv.at <= index;
		});
		return index - n_del_before + n_insert_before;
	};
	auto temp_polygons = mesh.polygons;
	for (auto& p: temp_polygons)
		for (auto& s: p.side)
			s.vertex = _remap(s.vertex);
	auto new_polygons = _new_polygons;
	for (auto& p: new_polygons)
		for (auto& s: p.p.side)
			s.vertex = _remap(s.vertex);

	// add/delete polygons
	{
		out.polygons.resize(mesh.polygons.num - _del_polygons.num + _new_polygons.num);
		int i_out = 0;
		// TODO optimize...
		for (int i=0; i<mesh.polygons.num; i++) { // in
			// insert new
			for (const auto& np: new_polygons)
				if (np.at == i) {
					inv.delete_polygon(i_out);
					out.polygons[i_out ++] = np.p;
				}

			if (_del_polygons.contains(i)) {
				// delete
				inv.add_polygon(temp_polygons[i], i_out);
			} else {
				// keep
				out.polygons[i_out ++] = temp_polygons[i];
			}
		}
		// append new
		for (const auto& np: new_polygons)
			if (np.at == -1 or np.at == mesh.polygons.num) {
				inv.delete_polygon(i_out);
				out.polygons[i_out ++] = np.p;
			}
	}

	mesh = out;
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

bool MeshEdit::changes_topology() const {
	if (_new_polygons.num > 0 or _del_polygons.num > 0 or _del_vertices.num > 0)
		return true;
	// TODO
	//for (const auto& p: _new_polygons)
	return true;
}


