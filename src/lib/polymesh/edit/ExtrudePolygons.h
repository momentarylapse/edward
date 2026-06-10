//
// Created by michi on 18.05.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

class PolygonMesh;
class MeshEdit;

namespace polymesh {
	MeshEdit extrude_polygons(const PolygonMesh& mesh, const base::set<int>& sel, float distance, bool keep_connected);
}

