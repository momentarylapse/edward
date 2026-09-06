/*
 * Renderer.cpp
 *
 *  Created on: Jan 4, 2020
 *      Author: michi
 */

#include "Renderer.h"
#include "Context.h"
#include <lib/math/rect.h>
#include <lib/math/vec2.h>
#include <lib/profiler/Profiler.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>

namespace yrenderer {

rect dynamicly_scaled_area(ygfx::FrameBuffer* fb, const vec2& scale) {
	return rect(0, max((float)fb->width * scale.x, 2.0f),
		0, max((float)fb->height * scale.y, 2.0f));
}

rect dynamicly_scaled_source(const vec2& scale) {
	return rect(0, scale.x, 0, scale.y);
}

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
	return {aspect_ratio.value_or(fb_ratio(frame_buffer)), true, false, frame_buffer, frame_buffer->area()};

}
RenderParams RenderParams::into_texture(ygfx::FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio) {
	return {aspect_ratio.value_or(fb_ratio(frame_buffer)), false, false, frame_buffer, frame_buffer->area()};
}


Node::Node(Context* _ctx, const string &name) {
	channel = profiler::create_channel(name, -1);
	custodian = this;

	ctx = _ctx;
	if (ctx)
		shader_manager = ctx->shader_manager;
}

Node::~Node() {
	profiler::delete_channel(channel);
}

void Node::add_child(Renderer* child) {
	custodian->children.add(child);
	profiler::set_parent(child->channel, channel);
}

void Node::remove_child(Renderer* child) {
	if (int i = children.find(child) >= 0) {
		profiler::set_parent(child->channel, -1);
		custodian->children.erase(i);
	}
}

void Node::add_sub_task(RenderTask* child) {
	sub_tasks.add(child);
	profiler::set_parent(child->channel, channel);
}

void Node::remove_sub_task(RenderTask* child) {
	if (int i = sub_tasks.find(child) >= 0) {
		profiler::set_parent(child->channel, -1);
		sub_tasks.erase(i);
	}
}

void Node::prepare_children(const RenderParams& params) {
	for (auto s: sub_tasks)
		if (s->active)
			s->render(params);
	for (auto c: children)
		c->prepare(params);
}



Renderer::Renderer(Context* _ctx, const string &name) : Node(_ctx, name) {
	ch_prepare = profiler::create_channel(name + ".p", channel);
}

Renderer::~Renderer() {
	profiler::delete_channel(ch_prepare);
}

void Renderer::prepare(const RenderParams& params) {
	prepare_children(params);
}

void Renderer::draw(const RenderParams& params) {
	for (auto c: children)
		c->draw(params);
}


RenderTask::RenderTask(Context* _ctx, const string& name) : Node(_ctx, name) {
}

RenderTask::~RenderTask() = default;


void show_render_tree(Node* r, int indent) {
	if (dynamic_cast<RenderTask*>(r))
		msg_write(string("  ").repeat(indent) + "T " + profiler::get_name(r->channel));
	else
		msg_write(string("  ").repeat(indent) + "R " + profiler::get_name(r->channel));
	for (auto c: r->sub_tasks)
		show_render_tree(c, indent + 1);
	for (auto c: r->children)
		show_render_tree(c, indent + 1);
}

}
