//
// Created by michi on 6/14/26.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

class PolygonMesh;
class MeshEdit;

namespace polymesh {
	MeshEdit auto_merge_polygons(const PolygonMesh& mesh, const base::set<int>& selp, float dang = 0.1f);
}
