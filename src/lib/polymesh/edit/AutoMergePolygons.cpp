//
// Created by michi on 6/14/26.
//

#include "AutoMergePolygons.h"
#include "../MeshEdit.h"
#include "../PolygonMesh.h"
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include "lib/os/msg.h"

namespace polymesh {
	bool try_merge_polygons(Polygon& p, const Polygon& p2) {
		for (int k=0; k<p.side.num; k++)
			for (int k2=0; k2<p2.side.num; k2++)
				if (p.side[k].vertex == p2.side[k2].vertex and p.side[(k+1)%p.side.num].vertex == p2.side[(k2+p2.side.num-1)%p2.side.num].vertex) {
				//	msg_write(str(p.get_vertices()));
				//	msg_write(str(p2.get_vertices()));
					for (int kk=1; kk<p2.side.num-1; kk++)
						p.side.insert(p2.side[(k2+kk)%p2.side.num], k+kk);
				//	msg_write(str(p.get_vertices()));
					return true;
				}
		return false;
	}

	void grow_polygon(const PolygonMesh& mesh, int i0, const Polygon& p0, base::set<int>& used, MeshEdit& ed, const base::set<int>& selp, float dang) {
		auto p = p0;
		const vec3 n = p.get_normal(mesh.vertices);
		base::set<int> used_now;
		bool found_new = false;
		do {
			found_new = false;
			for (const auto& [i2, p2]: enumerate(mesh.polygons))
				if (i2 != i0 and selp.contains(i2) and !used.contains(i2)) {
					if (vec3::dot(n, p2.get_normal(mesh.vertices)) < 1-dang)
						continue;
					if (try_merge_polygons(p, p2)) {
						used.add(i2);
						used_now.add(i2);
						found_new = true;
					}
				}
		} while (found_new);

		if (used_now.num == 0)
			return;

		ed.delete_polygon(i0);
		for (int i: used_now)
			ed.delete_polygon(i);
		ed.add_polygon(p);
	}


	MeshEdit auto_merge_polygons(const PolygonMesh& mesh, const base::set<int>& selp, float dang) {
		MeshEdit ed;
		const auto edges = mesh.edges();

		base::set<int> used;

		for (const auto& [i, p]: enumerate(mesh.polygons))
			if (selp.contains(i) and !used.contains(i)) {
				grow_polygon(mesh, i, p, used, ed, selp, dang);
			}

		return ed;
	}
} // polymesh
