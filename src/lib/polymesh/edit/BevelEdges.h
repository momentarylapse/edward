//
// Created by michi on 18.05.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

namespace polymesh {
	class Mesh;
	class MeshEdit;

	MeshEdit bevel_edges(const Mesh& mesh, const base::set<int>& selv, const base::set<int>& sele, float distance);
}
