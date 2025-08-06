/*
 * RegionRendererCommon.h
 *
 *  Created on: 06 Nov 2023
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>

#include <lib/math/rect.h>

namespace yrenderer {

class RegionRenderer : public Renderer {
public:
	explicit RegionRenderer(Context* ctx);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void add_region(Renderer *renderer, const rect &dest, int z);

	struct Region {
		rect dest;
		int z;
		Renderer *renderer;
	};

	Array<Region> regions;
	Array<Region*> sorted_regions;
};

}
