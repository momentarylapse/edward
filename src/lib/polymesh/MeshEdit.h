//
// Created by Michael Ankele on 2025-05-19.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>
#include "Mesh.h"

namespace polymesh {

struct MeshEdit {
	base::set<int> _del_vertices;
	base::set<int> _del_polygons;
	base::set<int> _del_spheres;
	base::set<int> _del_cylinders;
	struct NewVertex {
		Vertex v;
		int at;
		int id;
	};
	struct NewPolygon {
		Polygon p;
		int at;
	};
	struct NewSphere {
		Sphere p;
		int at;
	};
	struct NewCylinder {
		Cylinder p;
		int at;
	};
	Array<NewVertex> _new_vertices;
	Array<NewPolygon> _new_polygons; // indices are pre-vertex-insertion/deletion!
	Array<NewSphere> _new_spheres;
	Array<NewCylinder> _new_cylinders;
	void delete_vertex(int index);
	void delete_polygon(int index);
	void delete_sphere(int index);
	void delete_cylinder(int index);
	int add_vertex(const Vertex& v, int at_index=-1);
	void add_polygon(const Polygon& p, int at_index=-1);
	void add_sphere(const Sphere& s, int at_index=-1);
	void add_cylinder(const Cylinder& c, int at_index=-1);

	MeshEdit apply_inplace(Mesh& mesh) const;
	Mesh apply(const Mesh& mesh, MeshEdit* inv = nullptr) const;

	bool changes_topology() const;
};

}

