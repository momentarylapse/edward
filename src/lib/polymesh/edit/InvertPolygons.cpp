//
// Created by michi on 6/13/26.
//

#include "InvertPolygons.h"
#include "../MeshEdit.h"
#include "../Mesh.h"
#include <lib/base/iter.h>
#include <lib/base/sort.h>

namespace polymesh {
	MeshEdit invert_polygons(const Mesh& mesh, const base::set<int>& selp) {
		MeshEdit ed;

		for (const auto& [i, p]: enumerate(mesh.polygons))
			if (selp.contains(i)) {
				Polygon pp;
				pp.side = base::reverse(p.side);
				ed.add_polygon(pp);
				ed.delete_polygon(i);
			}

		return ed;
	}
} // polymesh