//
// Created by Michael Ankele on 2025-05-16.
//

#ifndef MESHRANDOMSURFACEPOINTS_H
#define MESHRANDOMSURFACEPOINTS_H

#include <lib/math/vec3.h>

struct PolygonMesh;

struct SurfacePoint {
	vec3 pos;
	vec3 normal;
};

Array<SurfacePoint> mesh_surface_points(const PolygonMesh& mesh, float r);



#endif //MESHRANDOMSURFACEPOINTS_H
