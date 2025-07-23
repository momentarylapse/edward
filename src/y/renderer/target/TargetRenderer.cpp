/*
 * TargetRenderer.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TargetRenderer.h"
#include <lib/profiler/Profiler.h>


TargetRenderer::TargetRenderer(const string &name) : Renderer(name) {
	ch_end = profiler::create_channel(name + ".end", channel);
}

void TargetRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	for (auto c: children) {
		c->draw(params);
	}
	profiler::end(channel);
}
