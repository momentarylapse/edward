//
// Created by Michael Ankele on 2025-02-19.
//

#include "ActionModelDeleteSelection.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include <mode_model/data/DataModel.h>
#include <mode_model/data/ModelMesh.h>

ActionModelDeleteSelection::ActionModelDeleteSelection(ModelMesh* m, const Data::Selection& selection, bool greedy) {
	mesh = m;
	if (greedy) {
		if (selection.contains(MultiViewType::MODEL_VERTEX)) {
			// delete all polygons with at least 1 selected vertex
			for (const auto& [i, o]: enumerate(mesh->polygons)) {
				bool del = false;
				for (const auto& s: o.side)
					if (selection[MultiViewType::MODEL_VERTEX].contains(s.vertex))
						del = true;
				if (del) {
					polygons.add(o);
					polygon_indices.add(i);
				}
			}
			for (const auto& [i, o]: enumerate(mesh->ball))
				if (selection[MultiViewType::MODEL_VERTEX].contains(o.index)) {
					spheres.add(o);
					sphere_indices.add(i);
				}
			for (const auto& [i, o]: enumerate(mesh->cylinder))
				if (selection[MultiViewType::MODEL_VERTEX].contains(o.index[0]) and selection[MultiViewType::MODEL_VERTEX].contains(o.index[1])) {
					cylinders.add(o);
					cylinder_indices.add(i);
				}

			// delete ALL selected vertices
			for (const auto& [i, o]: enumerate(mesh->vertices))
				if (selection[MultiViewType::MODEL_VERTEX].contains(i)) {
					vertices.add(o);
					vertex_indices.add(i);
				}
		}
	} else {
		// delete selected polygons
		if (selection.contains(MultiViewType::MODEL_POLYGON))
			for (const auto& [i, o]: enumerate(mesh->polygons))
				if (selection[MultiViewType::MODEL_POLYGON].contains(i)) {
					polygons.add(o);
					polygon_indices.add(i);
				}
		if (selection.contains(MultiViewType::MODEL_BALL))
			for (const auto& [i, o]: enumerate(mesh->ball))
				if (selection[MultiViewType::MODEL_BALL].contains(i)) {
					spheres.add(o);
					sphere_indices.add(i);
				}
		if (selection.contains(MultiViewType::MODEL_CYLINDER))
			for (const auto& [i, o]: enumerate(mesh->cylinder))
				if (selection[MultiViewType::MODEL_CYLINDER].contains(i)) {
					cylinders.add(o);
					cylinder_indices.add(i);
				}

		auto vertex_used_afterwards = [this, &selection] (int i) {
			for (const auto& [pi, p]: enumerate(mesh->polygons))
				if (!selection[MultiViewType::MODEL_POLYGON].contains(pi)) {
					for (const auto& s: p.side)
						if (s.vertex == i)
							return true;
				}
			for (const auto& [pi, p]: enumerate(mesh->ball))
				if (!selection[MultiViewType::MODEL_BALL].contains(pi))
					if (p.index == i)
						return true;
			for (const auto& [pi, p]: enumerate(mesh->cylinder))
				if (!selection[MultiViewType::MODEL_CYLINDER].contains(pi))
					if (p.index[0] == i or p.index[1] == 1)
						return true;
			return false;
		};

		// delete selected vertices IF UNUSED
		if (selection.contains(MultiViewType::MODEL_VERTEX))
			for (const auto& [i, o]: enumerate(mesh->vertices))
				if (selection[MultiViewType::MODEL_VERTEX].contains(i)) {
					if (vertex_used_afterwards(i))
						continue;
					vertices.add(o);
					vertex_indices.add(i);
				}
	}
}

void *ActionModelDeleteSelection::execute(Data*) {
	for (int i: base::reverse(vertex_indices))
		mesh->vertices.erase(i);
	for (int i: base::reverse(polygon_indices))
		mesh->polygons.erase(i);
	for (int i: base::reverse(sphere_indices))
		mesh->ball.erase(i);
	for (int i: base::reverse(cylinder_indices))
		mesh->cylinder.erase(i);

	for (auto& p: mesh->polygons)
		for (auto& s: p.side)
			s.vertex = map_vertex(s.vertex);
	for (auto& p: mesh->ball)
		p.index = map_vertex(p.index);
	for (auto& p: mesh->cylinder)
		for (int k=0; k<2; k++)
			p.index[k] = map_vertex(p.index[k]);
	return nullptr;
}

void ActionModelDeleteSelection::undo(Data*) {
	for (auto& p: mesh->polygons)
		for (auto& s: p.side)
			s.vertex = unmap_vertex(s.vertex);
	for (auto& p: mesh->ball)
		p.index = unmap_vertex(p.index);
	for (auto& p: mesh->cylinder)
		for (int k=0; k<2; k++)
			p.index[k] = unmap_vertex(p.index[k]);

	for (const auto& [ii, i]: enumerate(vertex_indices))
		mesh->vertices.insert(vertices[ii], i);
	for (const auto& [ii, i]: enumerate(polygon_indices))
		mesh->polygons.insert(polygons[ii], i);
	for (const auto& [ii, i]: enumerate(sphere_indices))
		mesh->ball.insert(spheres[ii], i);
	for (const auto& [ii, i]: enumerate(cylinder_indices))
		mesh->cylinder.insert(cylinders[ii], i);
}

int ActionModelDeleteSelection::map_vertex(int v) const {
	for (int i: base::reverse(vertex_indices))
		if (i < v)
			v --;
	return v;
}

int ActionModelDeleteSelection::unmap_vertex(int v) const {
	for (int i: vertex_indices)
		if (i <= v)
			v ++;
	return v;
}


