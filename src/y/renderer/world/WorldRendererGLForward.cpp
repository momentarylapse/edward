/*
 * WorldRendererGLForward.cpp
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#include "WorldRendererGLForward.h"
#include "pass/ShadowRendererGL.h"
#include "geometry/GeometryRendererGL.h"

#ifdef USING_OPENGL
#include "../base.h"
#include "../helper/jitter.h"
#include <lib/nix/nix.h>
#include <lib/image/image.h>
#include <lib/os/msg.h>

#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../helper/Scheduler.h"
#include "../../plugins/PluginManager.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../world/World.h"
#include "../../y/Entity.h"
#include "../../Config.h"
#include "../../meta.h"

// https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing


WorldRendererGLForward::WorldRendererGLForward(Camera *cam) : WorldRendererGL("world", cam, RenderPathType::FORWARD) {

	resource_manager->load_shader_module("forward/module-surface.shader");

	create_more();
}

void WorldRendererGLForward::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);

	if (!scene_view.cam)
		scene_view.cam = cam_main;

	prepare_lights();
	geo_renderer->prepare(params);

	if (scene_view.shadow_index >= 0)
		shadow_renderer->render(scene_view);

	static int _frame = 0;
	_frame ++;
	if (_frame >= cube_update_rate) {
		render_into_cubemap(depth_cube.get(), scene_view.cube_map.get(), suggest_cube_map_pos());
		_frame = 0;
		prepare_lights();
	}

	PerformanceMonitor::end(ch_prepare);
}

void WorldRendererGLForward::draw(const RenderParams& params) {
	PerformanceMonitor::begin(ch_draw);
	gpu_timestamp_begin(ch_draw);

	auto fb = params.frame_buffer;
	bool flip_y = params.target_is_window;

	PerformanceMonitor::begin(ch_bg);

	auto m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
	if (config.antialiasing_method == AntialiasingMethod::TAA)
		 m *= jitter(fb->width, fb->height, 0);

	// skyboxes
	auto cam = scene_view.cam;
	float max_depth = cam->max_depth;
	cam->max_depth = 2000000;
	cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(m * cam->m_projection);

	nix::clear_color(world.background);
	nix::clear_z();
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);
	nix::set_wire(wireframe);

	geo_renderer->draw_skyboxes();
	PerformanceMonitor::end(ch_bg);


	// world
	PerformanceMonitor::begin(ch_world);
	cam->max_depth = max_depth;
	cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(m * cam->m_projection);

	nix::bind_uniform_buffer(1, scene_view.ubo_light.get());
	nix::set_view_matrix(cam->view_matrix());
	nix::set_z(true, true);
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);

	geo_renderer->draw_opaque();
	geo_renderer->draw_transparent(params);
	PerformanceMonitor::end(ch_world);

	//nix::set_scissor(rect::EMPTY);

	nix::set_cull(nix::CullMode::BACK);
	nix::set_front(nix::Orientation::CW);
	nix::set_wire(false);

	gpu_timestamp_end(ch_draw);
	PerformanceMonitor::end(ch_draw);
}

void WorldRendererGLForward::render_into_texture(FrameBuffer *fb, Camera *cam) {
	nix::bind_frame_buffer(fb);

	std::swap(scene_view.cam, cam);
	prepare_lights();
	draw(RenderParams::into_texture(fb, 1.0f));
	std::swap(scene_view.cam, cam);
}

#endif
