/*
 * Renderer.cpp
 *
 *  Created on: Jan 4, 2020
 *      Author: michi
 */

#include "Renderer.h"
#include "Context.h"
#include <lib/math/rect.h>
#include <lib/profiler/Profiler.h>
#include <lib/ygraphics/graphics-impl.h>

namespace yrenderer {

const RenderParams RenderParams::WHATEVER = {};

RenderParams RenderParams::with_target(ygfx::FrameBuffer *fb) const {
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

float fb_ratio(ygfx::FrameBuffer* fb) {
	return (float)fb->width / (float)fb->height;
}

RenderParams RenderParams::into_window(ygfx::FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio) {
	return {aspect_ratio.value_or(fb_ratio(frame_buffer)), true, frame_buffer, frame_buffer->area()};

}
RenderParams RenderParams::into_texture(ygfx::FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio) {
	return {aspect_ratio.value_or(fb_ratio(frame_buffer)), false, frame_buffer, frame_buffer->area()};
}


Renderer::Renderer(Context* _ctx, const string &name) {
	channel = profiler::create_channel(name, -1);
	ch_prepare = profiler::create_channel(name + ".p", channel);

	ctx = _ctx;
	if (ctx)
		shader_manager = ctx->shader_manager;
}


Renderer::~Renderer() = default;

void Renderer::add_child(Renderer *child) {
	children.add(child);
}

void Renderer::add_sub_task(RenderTask* child) {
	sub_tasks.add(child);
}

void Renderer::prepare(const RenderParams& params) {
	for (auto s: sub_tasks)
		s->render(params);
	for (auto c: children)
		c->prepare(params);
}

void Renderer::draw(const RenderParams& params) {
	for (auto c: children)
		c->draw(params);
}


RenderTask::RenderTask(Context* _ctx, const string& name) {
	channel = profiler::create_channel(name, -1);

	ctx = _ctx;
	if (ctx)
		shader_manager = ctx->shader_manager;
}

RenderTask::~RenderTask() = default;

void RenderTask::add_child(Renderer *child) {
	children.add(child);
}

void RenderTask::add_sub_task(RenderTask* child) {
	sub_tasks.add(child);
}

void RenderTask::prepare_children(const RenderParams& params) {
	for (auto s: sub_tasks)
		s->render(params);
	for (auto c: children)
		c->prepare(params);
}

}
