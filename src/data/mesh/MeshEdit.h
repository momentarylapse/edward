//
// Created by Michael Ankele on 2025-05-19.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>
#include "PolygonMesh.h"


struct MeshEdit {
	base::set<int> _del_vertices;
	base::set<int> _del_polygons;
	struct NewVertex {
		MeshVertex v;
		int at;
		int id;
	};
	struct NewPolygon {
		Polygon p;
		int at;
	};
	Array<NewVertex> _new_vertices;
	Array<NewPolygon> _new_polygons; // indices are pre-vertex-insertion/deletion!
	void delete_vertex(int index);
	void delete_polygon(int index);
	int add_vertex(const MeshVertex& v, int at_index=-1);
	void add_polygon(const Polygon& p, int at_index=-1);

	MeshEdit apply_inplace(PolygonMesh& mesh) const;
	PolygonMesh apply(const PolygonMesh& mesh, MeshEdit* inv = nullptr) const;

	bool changes_topology() const;
};

