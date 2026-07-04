//
// Created by michi on 6/15/26.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

namespace polymesh {
	struct Mesh;
	struct MeshEdit;

	MeshEdit triangulate_polygons(const Mesh& mesh, const base::set<int>& selp);
}
