/*
 * TargetRenderer.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TargetRenderer.h"
#include "../../helper/PerformanceMonitor.h"


TargetRenderer::TargetRenderer(const string &name) : Renderer(name) {
	ch_end = PerformanceMonitor::create_channel(name + ".end", channel);
}

void TargetRenderer::draw(const RenderParams& params) {
	PerformanceMonitor::begin(channel);
	for (auto c: children) {
		c->draw(params);
	}
	PerformanceMonitor::end(channel);
}
