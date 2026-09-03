/*
 * RegionRenderer.cpp
 *
 *  Created on: 06 Nov 2023
 *      Author: michi
 */

#include "RegionRenderer.h"
#include <lib/profiler/Profiler.h>
#include <lib/base/algo.h>

namespace yrenderer {

static rect relative_area(const rect& a0, const rect& rel) {
	return {
		a0.x1 + a0.width() * rel.x1,
		a0.x1 + a0.width() * rel.x2,
		a0.y1 + a0.height() * rel.y1,
		a0.y1 + a0.height() * rel.y2};
}

RegionRenderer::RegionRenderer(Context* ctx) : Renderer(ctx, "rgn") {
}

void RegionRenderer::prepare(const RenderParams& params) {
	profiler::begin(ch_prepare);
	for (auto r: sorted_regions) {
		if (r->renderer) {
			auto sub_params = params.with_area(relative_area(params.area, r->dest));
			sub_params.desired_aspect_ratio *= r->dest.width() / r->dest.height();
			r->renderer->prepare(sub_params);
		}
	}
	profiler::end(ch_prepare);
}

void RegionRenderer::add_region(Renderer *renderer, const rect &dest, int z) {
	add_child(renderer);
	regions.add({dest, z, renderer});
	update_regions();
}

void RegionRenderer::remove_region(Renderer* renderer) {
	base::remove_if(regions, [renderer] (const Region& r) {
		return r.renderer == renderer;
	});
	remove_child(renderer);
	update_regions();
}

RegionRenderer::Region *RegionRenderer::get_region(Renderer *renderer) {
	for (auto& r: regions)
		if (r.renderer == renderer)
			return &r;
	return nullptr;
}

void RegionRenderer::update_regions() {
	// resort
	sorted_regions.clear();
	for (auto &r: regions)
		sorted_regions.add(&r);
	for (int i=0; i<sorted_regions.num; i++)
		for (int k=i+1; k<sorted_regions.num; k++)
			if (sorted_regions[i]->z > sorted_regions[k]->z)
				sorted_regions.swap(i, k);
}

}



