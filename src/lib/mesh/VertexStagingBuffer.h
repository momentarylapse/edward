//
// Created by Michael Ankele on 2025-02-19.
//

#pragma once

#include <lib/base/base.h>
#include <lib/ygraphics/graphics-fwd.h>
#include "Polygon.h"

struct vec3;

class VertexStagingBuffer {
public:
	Array<vec3> p, n;
	Array<float> uv[POLYGON_MAX_TEXTURES];
	void build(ygfx::VertexBuffer *vb, int num_textures);
	void clear();
};

