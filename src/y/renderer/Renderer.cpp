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
#include "../graphics-impl.h"


const RenderParams RenderParams::WHATEVER = {};

RenderParams RenderParams::with_target(FrameBuffer *fb) const {
	RenderParams r = *this;
	r.frame_buffer = fb;
	r.area = fb->area();
	r.target_is_window = false;
	return r;
}

RenderParams RenderParams::with_area(const rect& _area) const {
	RenderParams r = *this;
	r.area = _area;
	return r;
}

float fb_ratio(FrameBuffer* fb) {
	return (float)fb->width / (float)fb->height;
}

RenderParams RenderParams::into_window(FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio) {
	return {aspect_ratio.value_or(fb_ratio(frame_buffer)), true, frame_buffer, frame_buffer->area()};

}
RenderParams RenderParams::into_texture(FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio) {
	return {aspect_ratio.value_or(fb_ratio(frame_buffer)), false, frame_buffer, frame_buffer->area()};
}


Renderer::Renderer(const string &name) {
	channel = PerformanceMonitor::create_channel(name, -1);
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

void Renderer::draw(const RenderParams& params) {
	for (auto c: children)
		c->draw(params);
}

RenderTask::RenderTask(const string& name) : Renderer(name) {

}

