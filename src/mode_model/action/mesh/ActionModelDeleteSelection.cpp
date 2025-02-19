//
// Created by Michael Ankele on 2025-02-19.
//

#include "ActionModelDeleteSelection.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include <mode_model/data/DataModel.h>
#include <mode_model/data/ModelMesh.h>

ActionModelDeleteSelection::ActionModelDeleteSelection(DataModel* m, const Data::Selection& selection, bool greedy) {
	if (greedy) {
		if (selection.contains(MultiViewType::MODEL_VERTEX)) {
			// delete all polygons with at least 1 selected vertex
			for (const auto& [i, o]: enumerate(m->mesh->polygons)) {
				bool del = false;
				for (const auto& s: o.side)
					if (selection[MultiViewType::MODEL_VERTEX].contains(s.vertex))
						del = true;
				if (del) {
					polygons.add(o);
					polygon_indices.add(i);
				}
			}

			// delete ALL selected vertices
			for (const auto& [i, o]: enumerate(m->mesh->vertices))
				if (selection[MultiViewType::MODEL_VERTEX].contains(i)) {
					vertices.add(o);
					vertex_indices.add(i);
				}
		}
	} else {
		// delete selected polygons
		if (selection.contains(MultiViewType::MODEL_POLYGON))
			for (const auto& [i, o]: enumerate(m->mesh->polygons))
				if (selection[MultiViewType::MODEL_POLYGON].contains(i)) {
					polygons.add(o);
					polygon_indices.add(i);
				}

		auto vertex_used_afterwards = [m, &selection] (int i) {
			for (const auto& [pi, p]: enumerate(m->mesh->polygons))
				if (!selection[MultiViewType::MODEL_POLYGON].contains(pi)) {
					for (const auto& s: p.side)
						if (s.vertex == i)
							return true;
				}
			return false;
		};

		// delete selected vertices IF UNUSED
		if (selection.contains(MultiViewType::MODEL_VERTEX))
			for (const auto& [i, o]: enumerate(m->mesh->vertices))
				if (selection[MultiViewType::MODEL_VERTEX].contains(i)) {
					if (vertex_used_afterwards(i))
						continue;
					vertices.add(o);
					vertex_indices.add(i);
				}
	}
}

void *ActionModelDeleteSelection::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	for (int i: base::reverse(vertex_indices))
		m->mesh->vertices.erase(i);
	for (int i: base::reverse(polygon_indices))
		m->mesh->polygons.erase(i);

	for (auto& p: m->mesh->polygons)
		for (auto& s: p.side)
			s.vertex = map_vertex(s.vertex);
	return nullptr;
}

void ActionModelDeleteSelection::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);

	for (auto& p: m->mesh->polygons)
		for (auto& s: p.side)
			s.vertex = unmap_vertex(s.vertex);

	for (const auto& [ii, i]: enumerate(vertex_indices))
		m->mesh->vertices.insert(vertices[ii], i);
	for (const auto& [ii, i]: enumerate(polygon_indices))
		m->mesh->polygons.insert(polygons[ii], i);
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


