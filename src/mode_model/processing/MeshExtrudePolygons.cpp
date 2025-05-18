//
// Created by michi on 18.05.25.
//

#include "MeshExtrudePolygons.h"
#include <Session.h>
#include <data/mesh/Polygon.h>
#include <data/mesh/PolygonMesh.h>
#include <lib/base/iter.h>


MeshEdit mesh_prepare_extrude_polygons(const PolygonMesh& mesh, const Data::Selection& sel, float distance, bool keep_connected) {
	MeshEdit ed;
	for (const auto& [i, p]: enumerate(mesh.polygons))
		if (sel[MultiViewType::MODEL_POLYGON].contains(i)) {
			ed.delete_polygon(i);
			Polygon pp = p;
			Array<int> new_vertices;
			for (int k=0; k<p.side.num; k++) {
				int v0 = p.side[k].vertex;
				MeshVertex vv = mesh.vertices[v0];
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
				ed.add_polygon(ppp);
			}
			ed.add_polygon(pp);
		}
	return ed;
}
