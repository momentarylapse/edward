//
// Created by michi on 6/13/26.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

namespace polymesh {
	class Mesh;
	class MeshEdit;

	MeshEdit invert_polygons(const Mesh& mesh, const base::set<int>& selp);
}
