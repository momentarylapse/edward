/*
 * TargetRenderer.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TargetRenderer.h"
#include <lib/profiler/Profiler.h>


namespace yrenderer {

TargetRenderer::TargetRenderer(Context* ctx, const string &name) : Renderer(ctx, name) {
	ch_end = profiler::create_channel(name + ".end", channel);
}

void TargetRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	for (auto c: children) {
		c->draw(params);
	}
	profiler::end(channel);
}

}
