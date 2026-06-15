//
// Created by Michael Ankele on 2025-05-19.
//

#include "MeshEdit.h"

#include <lib/base/algo.h>
#include <lib/base/map.h>
#include <lib/base/sort.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>

namespace polymesh {

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

void MeshEdit::delete_sphere(int index) {
	_del_spheres.add(index);
}

void MeshEdit::delete_cylinder(int index) {
	_del_cylinders.add(index);
}

int MeshEdit::add_vertex(const Vertex& v, int at_index) {
	int id = -_new_vertices.num - 1; // starts at -1!
	_new_vertices.add({v, at_index, id});
	return id;
}

void MeshEdit::add_polygon(const Polygon& p, int at_index) {
	_new_polygons.add({p, at_index});
	_new_polygons.back().p.normal_dirty = true;
	_new_polygons.back().p.triangulation_dirty = true;
}

void MeshEdit::add_sphere(const Sphere &s, int at_index) {
	_new_spheres.add({s, at_index});
}

void MeshEdit::add_cylinder(const Cylinder &c, int at_index) {
	_new_cylinders.add({c, at_index});
}

MeshEdit MeshEdit::apply_inplace(Mesh& mesh) const {
	MeshEdit inv;
	Mesh out;
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

	auto temp_spheres = mesh.spheres;
	for (auto& s: temp_spheres)
		s.index = _remap(s.index);
	auto new_spheres = _new_spheres;
	for (auto& s: new_spheres)
		s.p.index = _remap(s.p.index);

	auto temp_cylinders = mesh.cylinders;
	for (auto& c: temp_cylinders)
		for (int k=0; k<2; k++)
			c.index[k] = _remap(c.index[k]);
	auto new_cylinders = _new_cylinders;
	for (auto& c: new_cylinders)
		for (int k=0; k<2; k++)
			c.p.index[k] = _remap(c.p.index[k]);

	// add/delete polygons

#define ADD_DELETE(WHAT) \
	{ \
		out.WHAT##s.resize(mesh.WHAT##s.num - _del_##WHAT##s.num + _new_##WHAT##s.num); \
		int i_out = 0; \
		/* TODO optimize... */ \
		for (int i=0; i<mesh.WHAT##s.num; i++) { /* in */ \
			/* insert new */ \
			for (const auto& np: new_##WHAT##s) \
				if (np.at == i) { \
					inv.delete_##WHAT(i_out); \
					out.WHAT##s[i_out ++] = np.p; \
				} \
			if (_del_##WHAT##s.contains(i)) { \
				/* delete */ \
				inv.add_##WHAT(temp_##WHAT##s[i], i_out); \
			} else { \
				/* keep */ \
				out.WHAT##s[i_out ++] = temp_##WHAT##s[i]; \
			} \
		} \
		/* append new */ \
		for (const auto& np: new_##WHAT##s) \
			if (np.at == -1 or np.at == mesh.WHAT##s.num) { \
				inv.delete_##WHAT(i_out); \
				out.WHAT##s[i_out ++] = np.p; \
			} \
	}
	ADD_DELETE(polygon);
	ADD_DELETE(sphere);
	ADD_DELETE(cylinder);

	mesh = out;
	return inv;
}

Mesh MeshEdit::apply(const Mesh& mesh, MeshEdit* inv) const {
	Mesh mesh2 = mesh;
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

}


