/*
 * RegionRendererGL.cpp
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#include "../regions/RegionRendererGL.h"

#ifdef USING_OPENGL
#include "../base.h"
#include <lib/nix/nix.h>
#include <helper/PerformanceMonitor.h>


void RegionRendererGL::draw(const RenderParams& params) {
	PerformanceMonitor::begin(ch_draw);

	const rect area = params.frame_buffer->area();

	for (auto r: sorted_regions) {
		if (r->renderer) {
			auto sub_params = params;
			sub_params.desired_aspect_ratio *= r->dest.width() / r->dest.height();
			auto rr = rect(area.x2 * r->dest.x1, area.x2 * r->dest.x2, area.y2 * r->dest.y1, area.y2 * r->dest.y2);
			nix::set_viewport(rect(rr));
			nix::set_scissor(rr);
			r->renderer->draw(sub_params);
		}
	}
	nix::set_scissor(rect::EMPTY);
	nix::set_viewport(area);
	PerformanceMonitor::end(ch_draw);
}

#endif


