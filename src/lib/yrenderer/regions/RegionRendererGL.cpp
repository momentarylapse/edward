/*
 * RegionRendererGL.cpp
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#include "RegionRenderer.h"

#ifdef USING_OPENGL
#include <lib/yrenderer/Context.h>
#include <lib/nix/nix.h>
#include <lib/profiler/Profiler.h>

namespace yrenderer {

void RegionRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);

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
	profiler::end(channel);
}

}

#endif


