//
// Created by Michael Ankele on 2025-02-19.
//

#include "ActionModelDeleteSelection.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include <mode_model/data/DataModel.h>
#include <mode_model/data/ModelMesh.h>

ActionModelDeleteSelection::ActionModelDeleteSelection(ModelMesh* m, const Selection& selection, bool greedy) {
	mesh = m;
	if (greedy) {
		if (selection.contains(MultiViewType::MODEL_VERTEX)) {
			// delete all polygons with at least 1 selected vertex
			for (const auto& [i, o]: enumerate(mesh->polygons)) {
				bool del = false;
				for (const auto& s: o.side)
					if (selection[MultiViewType::MODEL_VERTEX].contains(s.vertex))
						del = true;
				if (del)
					edit.delete_polygon(i);
			}
			for (const auto& [i, o]: enumerate(mesh->spheres))
				if (selection[MultiViewType::MODEL_VERTEX].contains(o.index))
					edit.delete_sphere(i);
			for (const auto& [i, o]: enumerate(mesh->cylinders))
				if (selection[MultiViewType::MODEL_VERTEX].contains(o.index[0]) and selection[MultiViewType::MODEL_VERTEX].contains(o.index[1]))
					edit.delete_cylinder(i);

			// delete ALL selected vertices
			for (const auto& [i, o]: enumerate(mesh->vertices))
				if (selection[MultiViewType::MODEL_VERTEX].contains(i))
					edit.delete_vertex(i);
		}
	} else {
		// delete selected polygons
		if (selection.contains(MultiViewType::MODEL_POLYGON))
			for (const auto& [i, o]: enumerate(mesh->polygons))
				if (selection[MultiViewType::MODEL_POLYGON].contains(i))
					edit.delete_polygon(i);
		if (selection.contains(MultiViewType::MODEL_BALL))
			for (const auto& [i, o]: enumerate(mesh->spheres))
				if (selection[MultiViewType::MODEL_BALL].contains(i))
					edit.delete_sphere(i);
		if (selection.contains(MultiViewType::MODEL_CYLINDER))
			for (const auto& [i, o]: enumerate(mesh->cylinders))
				if (selection[MultiViewType::MODEL_CYLINDER].contains(i))
					edit.delete_cylinder(i);

		auto vertex_used_afterwards = [this, &selection] (int i) {
			for (const auto& [pi, p]: enumerate(mesh->polygons))
				if (!selection[MultiViewType::MODEL_POLYGON].contains(pi)) {
					for (const auto& s: p.side)
						if (s.vertex == i)
							return true;
				}
			for (const auto& [pi, p]: enumerate(mesh->spheres))
				if (!selection[MultiViewType::MODEL_BALL].contains(pi))
					if (p.index == i)
						return true;
			for (const auto& [pi, p]: enumerate(mesh->cylinders))
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
					edit.delete_vertex(i);
				}
	}
}

void *ActionModelDeleteSelection::execute(history::Data* data) {
	auto d = dynamic_cast<DataModel*>(data);
	edit = edit.apply_inplace(*mesh);

	d->out_mesh_edited(edit);
	return nullptr;
}

void ActionModelDeleteSelection::undo(history::Data* data) {
	execute(data);
}


