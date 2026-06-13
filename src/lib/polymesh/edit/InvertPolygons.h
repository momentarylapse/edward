//
// Created by michi on 6/13/26.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

class PolygonMesh;
class MeshEdit;

namespace polymesh {
	MeshEdit invert_polygons(const PolygonMesh& mesh, const base::set<int>& selp);
}
