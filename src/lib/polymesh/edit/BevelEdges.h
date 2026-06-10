//
// Created by michi on 18.05.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

class PolygonMesh;
class MeshEdit;

namespace polymesh {
	MeshEdit bevel_edges(const PolygonMesh& mesh, const base::set<int>& selv, const base::set<int>& sele, float distance);
}
