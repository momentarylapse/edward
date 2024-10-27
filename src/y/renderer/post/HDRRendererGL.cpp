/*
 * HDRRendererGL.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "HDRRendererGL.h"
#ifdef USING_OPENGL
#include "../base.h"
#include <lib/nix/nix.h>
#include <lib/math/vec2.h>
#include <lib/math/rect.h>
#include <lib/os/msg.h>
#include <lib/any/any.h>
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../Config.h"
#include "../../y/EngineData.h"
#include "../../world/Camera.h"

static float resolution_scale_x = 1.0f;
static float resolution_scale_y = 1.0f;

static int BLOOM_LEVEL_SCALE = 4;

void apply_shader_data(Shader *s, const Any &shader_data);

namespace nix {
	void resolve_multisampling(FrameBuffer *target, FrameBuffer *source);
}


HDRRendererGL::HDRRendererGL(Camera *_cam, int width, int height) : PostProcessorStage("hdr") {
	ch_post_blur = PerformanceMonitor::create_channel("blur", channel);
	ch_out = PerformanceMonitor::create_channel("out", channel);

	cam = _cam;

	_depth_buffer = new nix::DepthBuffer(width, height, "d24s8");
	if (config.antialiasing_method == AntialiasingMethod::MSAA) {
		msg_error("yes msaa");
		fb_main_ms = new nix::FrameBuffer({
			new nix::TextureMultiSample(width, height, 4, "rgba:f16"),
			//_depth_buffer});
			new nix::RenderBuffer(width, height, 4, "d24s8")});


		shader_resolve_multisample = resource_manager->load_shader("forward/resolve-multisample.shader");
	} else {
		msg_error("no msaa");
	}
	fb_main = new nix::FrameBuffer({
			new nix::Texture(width, height, "rgba:f16"),
			_depth_buffer});
			//new nix::RenderBuffer(width, height, "d24s8)});

	int bloomw = width, bloomh = height;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		bloomw /= BLOOM_LEVEL_SCALE;
		bloomh /= BLOOM_LEVEL_SCALE;
		bloom_levels[i].fb_temp = new nix::FrameBuffer({
			new nix::Texture(bloomw, bloomh, "rgba:f16")});
		bloom_levels[i].fb_out = new nix::FrameBuffer({
			new nix::Texture(bloomw, bloomh, "rgba:f16")});
		bloom_levels[i].fb_temp->color_attachments[0]->set_options("wrap=clamp");
		bloom_levels[i].fb_out->color_attachments[0]->set_options("wrap=clamp");
	}

	fb_main->color_attachments[0]->set_options("wrap=clamp,minfilter=nearest");
	fb_main->color_attachments[0]->set_options("magfilter=" + config.resolution_scale_filter);

	shader_blur = resource_manager->load_shader("forward/blur.shader");
	shader_out = resource_manager->load_shader("forward/hdr.shader");

	vb_2d = new nix::VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);
}

HDRRendererGL::~HDRRendererGL() = default;


void render_source_into_framebuffer(Renderer *r, FrameBuffer *fb, VertexBuffer *vb_2d, const RenderParams& params) {
	vb_2d->create_quad(rect::ID_SYM, dynamicly_scaled_source());

	nix::bind_frame_buffer(fb);
	nix::set_viewport(dynamicly_scaled_area(fb));

	for (auto c: r->children)
		c->draw(params);
}

void render_out_through_shader(Renderer *r, const Array<Texture*> &source, Shader *shader, const Any &data, VertexBuffer *vb_2d, const RenderParams& params) {

	bool flip_y = params.target_is_window;

	PerformanceMonitor::begin(r->ch_draw);
	gpu_timestamp_begin(r->ch_draw);

	nix::set_textures(source);
	nix::set_shader(shader);
	apply_shader_data(shader, data);
	/*shader->set_float("exposure", cam->exposure);
	shader->set_float("bloom_factor", cam->bloom_factor);
	shader->set_float("scale_x", resolution_scale_x);
	shader->set_float("scale_y", resolution_scale_y);*/
	nix::set_projection_matrix(flip_y ? mat4::scale(1,-1,1) : mat4::ID);
	nix::set_view_matrix(mat4::ID);
	nix::set_model_matrix(mat4::ID);
	nix::set_cull(nix::CullMode::NONE);

	nix::set_z(false, false);

	nix::draw_triangles(vb_2d);

	nix::set_cull(nix::CullMode::BACK);
	gpu_timestamp_end(r->ch_draw);
	PerformanceMonitor::end(r->ch_draw);
}

void HDRRendererGL::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	auto sub_params = params.with_target((config.antialiasing_method == AntialiasingMethod::MSAA) ? fb_main_ms.get() : fb_main.get());

	if (!cam)
		cam = cam_main;

	for (auto c: children)
		c->prepare(sub_params);


	if (config.antialiasing_method == AntialiasingMethod::MSAA) {
		render_source_into_framebuffer(this, fb_main_ms.get(), vb_2d.get(), sub_params);

		// resolve
		if (true) {
			shader_resolve_multisample->set_float("width", fb_main_ms->width);
			shader_resolve_multisample->set_float("height", fb_main_ms->height);
			process({fb_main_ms->color_attachments[0].get()}, fb_main.get(), shader_resolve_multisample.get());
		} else {
			// not sure, why this does not work... :(
			nix::resolve_multisampling(fb_main.get(), fb_main_ms.get());
		}

	} else {
		render_source_into_framebuffer(this, fb_main.get(), vb_2d.get(), sub_params);
	}

	PerformanceMonitor::begin(ch_post_blur);
	gpu_timestamp_begin(ch_post_blur);
	//float r = cam->bloom_radius * engine.resolution_scale_x;
	float r = 3;//max(5 * engine.resolution_scale_x, 2.0f);
	auto bloom_input = fb_main.get();
	float threshold = 1.0f;
	for (int i=0; i<4; i++) {
		process_blur(bloom_input, bloom_levels[i].fb_temp.get(), r*BLOOM_LEVEL_SCALE, threshold, {1,0});
		process_blur(bloom_levels[i].fb_temp.get(), bloom_levels[i].fb_out.get(), r, 0.0f, {0,1});
		bloom_input = bloom_levels[i].fb_out.get();
		r = 3;//max(5 * engine.resolution_scale_x, 3.0f);
		threshold = 0;
	}
	//glGenerateTextureMipmap(fb_small2->color_attachments[0]->texture);
	gpu_timestamp_end(ch_post_blur);
	PerformanceMonitor::end(ch_post_blur);
	PerformanceMonitor::end(ch_prepare);
}

void HDRRendererGL::draw(const RenderParams& params) {
	Any data;
	data.dict_set("exposure", cam->exposure);
	data.dict_set("bloom_factor", cam->bloom_factor);
	data.dict_set("scale_x", resolution_scale_x);
	data.dict_set("scale_y", resolution_scale_y);


	Array<Texture*> tex = {fb_main->color_attachments[0].get(), bloom_levels[0].fb_out->color_attachments[0].get(), bloom_levels[1].fb_out->color_attachments[0].get(), bloom_levels[2].fb_out->color_attachments[0].get(), bloom_levels[3].fb_out->color_attachments[0].get()};
	render_out_through_shader(this, tex, shader_out.get(), data, vb_2d.get(), params);
	//render_out(fb_main.get(), fb_small2->color_attachments[0].get());
}

void HDRRendererGL::process_blur(FrameBuffer *source, FrameBuffer *target, float r, float threshold, const vec2 &axis) {
	shader_blur->set_float("radius", r);
	shader_blur->set_float("threshold", threshold / cam->exposure);
	shader_blur->set_floats("axis", &axis.x, 2);
	process(weak(source->color_attachments), target, shader_blur.get());
}

void HDRRendererGL::process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader) {
	nix::bind_frame_buffer(target);
	nix::set_viewport(dynamicly_scaled_area(target));
	//nix::set_scissor(rect(0, target->width*resolution_scale_x, 0, target->height*resolution_scale_y));
	nix::set_z(false, false);
	//nix::set_projection_ortho_relative();
	//nix::set_view_matrix(matrix::ID);
	//nix::set_model_matrix(matrix::ID);
	shader->set_floats("resolution_scale", &resolution_scale_x, 2);
	nix::set_shader(shader);

	nix::set_textures(source);
	nix::draw_triangles(vb_2d.get());
	//nix::set_scissor(rect::EMPTY);
}

void HDRRendererGL::render_out(FrameBuffer *source, Texture *bloom, const RenderParams& params) {

	bool flip_y = params.target_is_window;

	PerformanceMonitor::begin(ch_out);
	gpu_timestamp_begin(ch_out);

	nix::set_textures({source->color_attachments[0].get(), bloom});
	nix::set_shader(shader_out.get());
	shader_out->set_float("exposure", cam->exposure);
	shader_out->set_float("bloom_factor", cam->bloom_factor);
	shader_out->set_float("scale_x", resolution_scale_x);
	shader_out->set_float("scale_y", resolution_scale_y);
	nix::set_projection_matrix(flip_y ? mat4::scale(1,-1,1) : mat4::ID);
	nix::set_view_matrix(mat4::ID);
	nix::set_model_matrix(mat4::ID);
	nix::set_cull(nix::CullMode::NONE);

	nix::set_z(false, false);

	nix::draw_triangles(vb_2d.get());

	nix::set_cull(nix::CullMode::BACK);
	gpu_timestamp_end(ch_out);
	PerformanceMonitor::end(ch_out);
}

#endif
