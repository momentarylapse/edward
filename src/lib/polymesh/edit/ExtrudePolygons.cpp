//
// Created by michi on 18.05.25.
//

#include "ExtrudePolygons.h"
#include <lib/polymesh/Polygon.h>
#include <lib/polymesh/Mesh.h>
#include <lib/polymesh/MeshEdit.h>
#include <lib/base/iter.h>

namespace polymesh {
	MeshEdit extrude_polygons(const Mesh& mesh, const base::set<int>& sel, float distance, bool keep_connected) {
		MeshEdit ed;
		for (const auto& [i, p]: enumerate(mesh.polygons))
			if (sel.contains(i)) {
				ed.delete_polygon(i);
				Polygon pp = p;
				Array<int> new_vertices;
				for (int k=0; k<p.side.num; k++) {
					int v0 = p.side[k].vertex;
					Vertex vv = mesh.vertices[v0];
					vv.pos += distance * p.get_normal(mesh.vertices);
					int v1 = ed.add_vertex(vv);
					new_vertices.add(v1);
					pp.side[k].vertex = v1;
				}
				for (int k=0; k<p.side.num; k++) {
					Polygon ppp;
					ppp.side.resize(4);
					ppp.side[0].vertex = p.side[k].vertex;
					ppp.side[1].vertex = p.side[(k+1) % p.side.num].vertex;
					ppp.side[2].vertex = new_vertices[(k+1) % p.side.num];
					ppp.side[3].vertex = new_vertices[k];
					ppp.side[0].uv = {0,0,0};
					ppp.side[1].uv = {1,0,0};
					ppp.side[2].uv = {1,1,0};
					ppp.side[3].uv = {0,1,0};
					ed.add_polygon(ppp);
				}
				ed.add_polygon(pp);
			}
		return ed;
	}
}
