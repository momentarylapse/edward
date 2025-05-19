//
// Created by Michael Ankele on 2025-05-19.
//

#ifndef MESHEDIT_H
#define MESHEDIT_H

#include <lib/base/base.h>
#include <lib/base/set.h>
#include "PolygonMesh.h"


struct MeshEdit {
	base::set<int> _del_vertices;
	base::set<int> _del_polygons;
	Array<MeshVertex> _new_vertices;
	Array<Polygon> _new_polygons; // indices are pre-vertex-insertion/deletion!
	void delete_vertex(int index);
	void delete_polygon(int index);
	int add_vertex(const MeshVertex& v);
	void add_polygon(const Polygon& p);

	MeshEdit apply_inplace(PolygonMesh& mesh) const;
	PolygonMesh apply(const PolygonMesh& mesh, MeshEdit* inv = nullptr) const;
};


#endif //MESHEDIT_H
