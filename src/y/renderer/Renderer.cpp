/*
 * Renderer.cpp
 *
 *  Created on: Jan 4, 2020
 *      Author: michi
 */

#include "Renderer.h"
#include "../y/EngineData.h"
#include "../lib/math/rect.h"
#include "../helper/PerformanceMonitor.h"
#include "world/WorldRenderer.h"

#include "../lib/os/msg.h"


const RenderParams RenderParams::WHATEVER = {};

RenderParams RenderParams::with_target(FrameBuffer *fb) const {
	RenderParams r = *this;
	r.frame_buffer = fb;
	r.target_is_window = false;
	return r;
}

RenderParams RenderParams::into_window(FrameBuffer *frame_buffer, float aspect_ratio) {
	return {aspect_ratio, true, frame_buffer};

}
RenderParams RenderParams::into_texture(FrameBuffer *frame_buffer, float aspect_ratio) {
	return {aspect_ratio, false, frame_buffer};
}


Renderer::Renderer(const string &name) {
	channel = PerformanceMonitor::create_channel(name, -1);
	ch_draw = PerformanceMonitor::create_channel(name + ".d", channel);
	ch_prepare = PerformanceMonitor::create_channel(name + ".p", channel);

	context = engine.context;
	resource_manager = engine.resource_manager;
}


Renderer::~Renderer() = default;

void Renderer::add_child(Renderer *child) {
	children.add(child);
}

void Renderer::prepare(const RenderParams& params) {
	for (auto c: children)
		c->prepare(params);
}

