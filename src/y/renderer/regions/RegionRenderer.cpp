/*
 * RegionRenderer.cpp
 *
 *  Created on: 06 Nov 2023
 *      Author: michi
 */

#include "RegionRenderer.h"
#include <helper/PerformanceMonitor.h>

RegionRenderer::RegionRenderer() : Renderer("rgn") {
}

void RegionRenderer::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	for (auto r: sorted_regions) {
		if (r->renderer) {
			auto sub_params = params;
			sub_params.desired_aspect_ratio *= r->dest.width() / r->dest.height();
			r->renderer->prepare(sub_params);
		}
	}
	PerformanceMonitor::end(ch_prepare);
}

void RegionRenderer::add_region(Renderer *renderer, const rect &dest, int z) {
	add_child(renderer);

	regions.add({dest, z, renderer});

	// resort
	sorted_regions.clear();
	for (auto &r: regions)
		sorted_regions.add(&r);
	for (int i=0; i<sorted_regions.num; i++)
		for (int k=i+1; k<sorted_regions.num; k++)
			if (sorted_regions[i]->z > sorted_regions[k]->z)
				sorted_regions.swap(i, k);
}



