//
// Created by Michael Ankele on 2025-05-16.
//

#include "MeshRandomSurfacePoints.h"
#include <data/mesh/PolygonMesh.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>


Array<SurfacePoint> mesh_surface_points(const PolygonMesh& mesh, float r) {
	Array<SurfacePoint> points;

	auto allowed = [&points, r] (const vec3& pos) {
		for (const auto& p: points)
			if ((pos - p.pos).length_sqr() < r*r)
				return false;
		return true;
	};

	for (const auto& p: mesh.polygons) {
		auto indices = p.triangulate(mesh.vertices);

		auto aa = p.get_area_vector(mesh.vertices);
		float area = aa.length();
		vec3 n = aa.normalized();

		int N = (int)(area / (pi * r * r));
		for (int i=0; i<N*4; i++) {
			for (int k=0; k<7; k++) {
				int nt = randi(indices.num / 3);
				vec2 fg = vec2(randf(1.0f), randf(1.0f));
				if (fg.x + fg.y > 1)
					fg = vec2(1, 1) - fg;
				const vec3 a = mesh.vertices[p.side[indices[nt*3]].vertex].pos;
				const vec3 b = mesh.vertices[p.side[indices[nt*3+1]].vertex].pos;
				const vec3 c = mesh.vertices[p.side[indices[nt*3+2]].vertex].pos;
				const vec3 pos = (1-fg.x-fg.y)*a + fg.x*b + fg.y*c;

				if (allowed(pos)) {
					points.add({pos, n});
					break;
				}
			}
		}
	}
	return points;
}
