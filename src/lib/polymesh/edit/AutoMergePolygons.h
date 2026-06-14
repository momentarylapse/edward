//
// Created by michi on 6/14/26.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/set.h>


namespace polymesh {
	class Mesh;
	class MeshEdit;

	MeshEdit auto_merge_polygons(const Mesh& mesh, const base::set<int>& selp, float dang = 0.1f);
}
