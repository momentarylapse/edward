/*
 * PostProcessorVulkan.cpp
 *
 *  Created on: Dec 7, 2021
 *      Author: michi
 */

#include "PostProcessorVulkan.h"

#ifdef USING_VULKAN
#include "../base.h"
#include "../../graphics-impl.h"
#include <lib/math/vec2.h>
#include <lib/math/rect.h>
#include <lib/os/msg.h>
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../Config.h"
#include "../../world/Camera.h"

static float resolution_scale_x = 1.0f;
static float resolution_scale_y = 1.0f;


PostProcessorVulkan::PostProcessorVulkan() {
	ch_post_blur = PerformanceMonitor::create_channel("blur", channel);
	ch_out = PerformanceMonitor::create_channel("out", channel);


	/*fb1 = new FrameBuffer({
		new vulkan::DynamicTexture(width, height, 1, "rgba:f16")});
	fb2 = new FrameBuffer({
		new vulkan::DynamicTexture(width, height, 1, "rgba:f16")});
	fb1->attachments[0]->set_options("wrap=clamp");
	fb2->attachments[0]->set_options("wrap=clamp");*/


	/*_depth_buffer = new nix::DepthBuffer(width, height, "d24s8");
	if (config.antialiasing_method == AntialiasingMethod::MSAA) {
		msg_error("yes msaa");
		fb_main = new nix::FrameBuffer({
			new nix::TextureMultiSample(width, height, 4, "rgba:f16"),
			//_depth_buffer});
			new nix::RenderBuffer(width, height, 4, "d24s8")});
	} else {
		msg_error("no msaa");
		fb_main = new nix::FrameBuffer({
			new nix::Texture(width, height, "rgba:f16"),
			_depth_buffer});
			//new nix::RenderBuffer(width, height, "d24s8)});
	}
	fb_small1 = new nix::FrameBuffer({
		new nix::Texture(width/2, height/2, "rgba:f16")});
	fb_small2 = new nix::FrameBuffer({
		new nix::Texture(width/2, height/2, "rgba:f16")});

	if (fb_main->color_attachments[0]->type != nix::Texture::Type::MULTISAMPLE)
		fb_main->color_attachments[0]->set_options("wrap=clamp,minfilter=nearest");
	fb_small1->color_attachments[0]->set_options("wrap=clamp");
	fb_small2->color_attachments[0]->set_options("wrap=clamp");

	shader_blur = ResourceManager::load_shader("forward/blur.shader");
	shader_out = ResourceManager::load_shader("forward/hdr.shader");*/

	vb_2d = new VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);


	//shader_depth = ResourceManager::load_shader("forward/depth.shader");
	//shader_resolve_multisample = ResourceManager::load_shader("forward/resolve-multisample.shader");
}

PostProcessorVulkan::~PostProcessorVulkan() {
}

FrameBuffer *PostProcessorVulkan::next_fb(FrameBuffer *cur) {
	return (cur == fb1) ? fb2.get() : fb1.get();
}

void PostProcessorVulkan::prepare(const RenderParams& params) {
	for (auto c: children)
		c->prepare(params);

	if (stages.num == 0)
		return;

	/*vb_2d->create_quad(rect::ID_SYM, dynamicly_scaled_source());

	nix::bind_frame_buffer(fb1.get());
	nix::set_viewport(dynamicly_scaled_area(fb1.get()));

	if (child)
		child->draw();*/

	//PerformanceMonitor::begin(ch_post_blur);
	//process_blur(fb_main.get(), fb_small1.get(), 1.0f, {2,0});
	//process_blur(fb_small1.get(), fb_small2.get(), 0.0f, {0,1});
	//PerformanceMonitor::end(ch_post_blur);
}

void PostProcessorVulkan::draw(const RenderParams& params) {
	if (stages.num == 0) {
		for (auto c: children)
			c->draw(params);
	} else {
		stages.back()->draw(params);
	}
	bool flip_y = params.target_is_window;
	//render_out(fb_main.get(), fb_small2->color_attachments[0].get(), flip_y);
}

// GTX750: 1920x1080 0.277 ms per trivial step
FrameBuffer* PostProcessorVulkan::do_post_processing(FrameBuffer *source) {
	//PerformanceMonitor::begin(ch_post);
	auto cur = source;

	// scripts
	for (auto *p: stages) {
		PerformanceMonitor::begin(p->channel);
//		cur = (*p->func)(cur);
		PerformanceMonitor::end(p->channel);
	}


#if 0
	if (cam->focus_enabled) {
		PerformanceMonitor::begin(ch_post_focus);
		auto next = next_fb(cur);
		process_depth(cur, next, complex(1,0));
		cur = next;
		next = next_fb(cur);
		process_depth(cur, next, complex(0,1));
		cur = next;
		PerformanceMonitor::end(ch_post_focus);
	}
#endif

	//PerformanceMonitor::end(ch_post);
	return cur;
}





FrameBuffer* PostProcessorVulkan::resolve_multisampling(FrameBuffer *source) {
	//auto next = next_fb(source);
	/*if (true) {
		shader_resolve_multisample->set_float("width", source->width);
		shader_resolve_multisample->set_float("height", source->height);
		process({source->color_attachments[0].get(), depth_buffer}, next, shader_resolve_multisample.get());
	} else {
		// not sure, why this does not work... :(
		nix::resolve_multisampling(next, source);
	}*/
	//return next;
	return source;
}


void PostProcessorVulkan::process_blur(FrameBuffer *source, FrameBuffer *target, float threshold, const vec2 &axis) {
	/*float r = cam->bloom_radius * resolution_scale_x;
	shader_blur->set_float("radius", r);
	shader_blur->set_float("threshold", threshold / cam->exposure);
	shader_blur->set_floats("axis", &axis.x, 2);
	process(weak(source->color_attachments), target, shader_blur.get());*/
}

void PostProcessorVulkan::process_depth(FrameBuffer *source, FrameBuffer *target, const vec2 &axis) {
	/*shader_depth->set_float("max_radius", 50);
	shader_depth->set_float("focal_length", cam->focal_length);
	shader_depth->set_float("focal_blur", cam->focal_blur);
	shader_depth->set_floats("axis", &axis.x, 2);
	shader_depth->set_matrix("invproj", cam->m_projection.inverse());*/
//	process({source->color_attachments[0].get(), depth_buffer()}, target, shader_depth.get());
}

void PostProcessorVulkan::process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader, const Any &data) {
	/*nix::bind_frame_buffer(target);
	nix::set_scissor(dynamicly_scaled_area(target));
	nix::set_z(false, false);
	//nix::set_projection_ortho_relative();
	//nix::set_view_matrix(matrix::ID);
	//nix::set_model_matrix(matrix::ID);
	float resolution_scale_x = 1.0f;
	shader->set_floats("resolution_scale", &resolution_scale_x, 2);
	nix::set_shader(shader);

	nix::set_textures(source);
	nix::draw_triangles(vb_2d);
	nix::set_scissor(rect::EMPTY);*/
}

#endif
