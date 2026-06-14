//
// Created by michi on 18.05.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>

namespace polymesh {
	class Mesh;
	class MeshEdit;

	MeshEdit extrude_polygons(const Mesh& mesh, const base::set<int>& sel, float distance, bool keep_connected);
}

