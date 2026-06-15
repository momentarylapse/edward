//
// Created by michi on 6/15/26.
//

#include "TriangulatePolygons.h"
#include "../MeshEdit.h"
#include "../Mesh.h"
#include <lib/base/iter.h>

namespace polymesh {
	MeshEdit triangulate_polygons(const Mesh& mesh, const base::set<int>& selp) {
		MeshEdit ed;

		for (const auto& [i, p]: enumerate(mesh.polygons))
			if (selp.contains(i) and p.side.num > 3) {
				auto vv = p.triangulate(mesh.vertices);
				for (int k=0; k<vv.num; k+=3) {
					Polygon pp;
					pp.side.add(p.side[vv[k]]);
					pp.side.add(p.side[vv[k+1]]);
					pp.side.add(p.side[vv[k+2]]);
					ed.add_polygon(pp);
				}
				ed.delete_polygon(i);
			}

		return ed;
	}
} // polymesh