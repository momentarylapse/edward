/*
 * WorldRendererGL.cpp
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#include "WorldRendererGLDeferred.h"

#ifdef USING_OPENGL
#include "geometry/GeometryRendererGL.h"
#include "pass/ShadowRendererGL.h"
#include "../base.h"
#include <lib/nix/nix.h>
#include <lib/os/msg.h>
#include <lib/math/random.h>
#include <lib/math/vec4.h>

#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../plugins/ControllerManager.h"
#include "../../plugins/PluginManager.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../world/World.h"
#include "../../y/Entity.h"
#include "../../Config.h"
#include "../../meta.h"
#include "../../graphics-impl.h"


WorldRendererGLDeferred::WorldRendererGLDeferred(Camera *cam, int width, int height) : WorldRendererGL("world/def", cam, RenderPathType::DEFERRED) {

	gbuffer = new nix::FrameBuffer({
		new nix::Texture(width, height, "rgba:f16"), // diffuse
		new nix::Texture(width, height, "rgba:f16"), // emission
		new nix::Texture(width, height, "rgba:f16"), // pos
		new nix::Texture(width, height, "rgba:f16"), // normal,reflection
		new nix::DepthBuffer(width, height, "d24s8")});

	for (auto a: gbuffer->color_attachments)
		a->set_options("wrap=clamp,magfilter=nearest,minfilter=nearest");


	resource_manager->load_shader_module("forward/module-surface.shader");
	resource_manager->load_shader_module("deferred/module-surface.shader");

	shader_gbuffer_out = resource_manager->load_shader("deferred/out.shader");
	if (!shader_gbuffer_out->link_uniform_block("SSAO", 13))
		msg_error("SSAO");

	ssao_sample_buffer = new nix::UniformBuffer();
	Array<vec4> ssao_samples;
	Random r;
	for (int i=0; i<64; i++) {
		auto v = r.dir() * pow(r.uniform01(), 1);
		ssao_samples.add(vec4(v.x, v.y, abs(v.z), 0));
	}
	ssao_sample_buffer->update_array(ssao_samples);

	ch_gbuf_out = PerformanceMonitor::create_channel("gbuf-out", channel);
	ch_trans = PerformanceMonitor::create_channel("trans", channel);

	create_more();

	geo_renderer_trans = new GeometryRendererGL(RenderPathType::FORWARD, scene_view);
	add_child(geo_renderer_trans.get());
}

void WorldRendererGLDeferred::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);

	if (!scene_view.cam)
		scene_view.cam = cam_main;

	auto sub_params = params.with_target(gbuffer.get());

	scene_view.check_terrains(cam_main->owner->pos);
	prepare_lights(cam_main, main_rvd);

	geo_renderer->prepare(sub_params);
	geo_renderer_trans->prepare(params); // keep drawing into direct target

	if (scene_view.shadow_index >= 0)
		shadow_renderer->render(scene_view);

	render_into_gbuffer(gbuffer.get(), sub_params);

	//auto source = do_post_processing(fb_main.get());

	PerformanceMonitor::end(ch_prepare);
}

void WorldRendererGLDeferred::draw(const RenderParams& params) {
	PerformanceMonitor::begin(ch_draw);
	gpu_timestamp_begin(ch_draw);

	auto target = params.frame_buffer;

	draw_background(target, params);

	render_out_from_gbuffer(gbuffer.get(), params);

	PerformanceMonitor::begin(ch_trans);
	bool flip_y = params.target_is_window;
	mat4 m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
	auto cam = scene_view.cam;
	cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(m * cam->m_projection);
	nix::bind_uniform_buffer(1, main_rvd.ubo_light.get());
	nix::set_view_matrix(cam->view_matrix());
	nix::set_z(true, true);
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);

	geo_renderer_trans->draw_transparent(params, main_rvd);
	nix::set_cull(nix::CullMode::BACK);
	nix::set_front(nix::Orientation::CW);

	nix::set_z(false, false);
	nix::set_projection_matrix(mat4::ID);
	nix::set_view_matrix(mat4::ID);
	PerformanceMonitor::end(ch_trans);

	gpu_timestamp_end(ch_draw);
	PerformanceMonitor::end(ch_draw);
}

void WorldRendererGLDeferred::draw_background(nix::FrameBuffer *fb, const RenderParams& params) {
	PerformanceMonitor::begin(ch_bg);

	auto cam = scene_view.cam;
//	float max_depth = cam->max_depth;
	cam->max_depth = 2000000;
	bool flip_y = params.target_is_window;
	mat4 m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
	cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(m * cam->m_projection);

	//nix::clear_color(Green);
	nix::clear_color(world.background);

	geo_renderer->draw_skyboxes(params, main_rvd);
	PerformanceMonitor::end(ch_bg);

}

void WorldRendererGLDeferred::render_out_from_gbuffer(nix::FrameBuffer *source, const RenderParams& params) {
	PerformanceMonitor::begin(ch_gbuf_out);
	auto s = shader_gbuffer_out.get();
	if (geo_renderer->using_view_space)
		s->set_floats("eye_pos", &vec3::ZERO.x, 3);
	else
		s->set_floats("eye_pos", &scene_view.cam->owner->pos.x, 3); // NAH
	s->set_int("num_lights", scene_view.lights.num);
	s->set_int("shadow_index", scene_view.shadow_index);
	s->set_float("ambient_occlusion_radius", config.ambient_occlusion_radius);
	nix::bind_uniform_buffer(13, ssao_sample_buffer);

	nix::bind_uniform_buffer(1, main_rvd.ubo_light.get());
	auto tex = weak(source->color_attachments);
	tex.add(source->depth_buffer.get());
	tex.add(scene_view.fb_shadow1->depth_buffer.get());
	tex.add(scene_view.fb_shadow2->depth_buffer.get());
	nix::bind_textures(tex);


	nix::set_z(false, false);
	nix::set_front(nix::Orientation::CW);
	nix::set_cull(nix::CullMode::NONE);
	float resolution_scale_x = 1.0f;
	s->set_floats("resolution_scale", &resolution_scale_x, 2);
	nix::set_shader(s);

	context->vb_temp->create_quad(rect::ID_SYM, dynamicly_scaled_source());
	nix::draw_triangles(context->vb_temp);

	// ...
	//geo_renderer->draw_transparent();

	PerformanceMonitor::end(ch_gbuf_out);
}

//void WorldRendererGLDeferred::render_into_texture(nix::FrameBuffer *fb, Camera *cam) {}

void WorldRendererGLDeferred::render_into_gbuffer(nix::FrameBuffer *fb, const RenderParams& params) {
	PerformanceMonitor::begin(ch_world);
	gpu_timestamp_begin(ch_world);
	nix::bind_frame_buffer(fb);
	nix::set_viewport(dynamicly_scaled_area(fb));

	//nix::clear_color(Green);//world.background);
	nix::clear_z();
	//fb->clear_color(2, color(0, 0,0,max_depth * 0.99f));
	fb->clear_color(0, color(-1, 0,1,0));


	auto cam = scene_view.cam;
	cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(mat4::scale(1,1,1) * cam->m_projection);

	nix::set_cull(nix::CullMode::BACK);
	nix::set_front(nix::Orientation::CCW);

	geo_renderer->draw_opaque(params, main_rvd);
	ControllerManager::handle_render_inject();

	nix::set_cull(nix::CullMode::BACK);
	nix::set_front(nix::Orientation::CCW);
	geo_renderer->draw_particles(params, main_rvd);
	gpu_timestamp_end(ch_world);
	PerformanceMonitor::end(ch_world);
}


#endif
