/*
 * WorldRendererVulkanForward.cpp
 *
 *  Created on: Nov 18, 2021
 *      Author: michi
 */

#include "WorldRendererVulkanForward.h"
#ifdef USING_VULKAN
#include "pass/ShadowRendererVulkan.h"
#include "../../graphics-impl.h"
#include "../base.h"
#include <lib/os/msg.h>

#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../gui/gui.h"
#include "../../gui/Node.h"
#include "../../world/Camera.h"
#include "../../world/World.h"


WorldRendererVulkanForward::WorldRendererVulkanForward(vulkan::Device *_device, Camera *cam) : WorldRendererVulkan("fw", cam, RenderPathType::FORWARD) {
	device = _device;

	create_more();
}

void WorldRendererVulkanForward::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	if (!scene_view.cam)
		scene_view.cam = cam_main;

	auto cb = params.command_buffer;


#ifndef OS_MAC
	static int _frame = 0;
	_frame ++;
	if (_frame >= cube_update_rate) {
		render_into_cubemap(params, scene_view.cube_map.get(), suggest_cube_map_pos());
		_frame = 0;
	}
#endif

	scene_view.cam->update_matrices(params.desired_aspect_ratio);

	prepare_lights(scene_view.cam, geo_renderer->rvd_def);
	
	geo_renderer->prepare(params);

	if (scene_view.shadow_index >= 0)
		shadow_renderer->render(cb, scene_view);

	PerformanceMonitor::end(ch_prepare);
}

void WorldRendererVulkanForward::draw(const RenderParams& params) {
	auto cb = params.command_buffer;
	auto rp = params.render_pass;

	PerformanceMonitor::begin(ch_draw);
	gpu_timestamp_begin(cb, ch_draw);

	auto &rvd = geo_renderer->rvd_def;

	cb->clear({world.background}, 1.0f);
	geo_renderer->draw_skyboxes(cb, rp, params.desired_aspect_ratio, rvd);

	UBO ubo;
	ubo.p = scene_view.cam->m_projection;
	ubo.v = scene_view.cam->m_view;
	ubo.num_lights = scene_view.lights.num;
	ubo.shadow_index = scene_view.shadow_index;

	geo_renderer->draw_terrains(cb, rp, ubo, rvd);
	geo_renderer->draw_objects_opaque(cb, rp, ubo, rvd);
	geo_renderer->draw_objects_instanced(cb, rp, ubo, rvd);
	geo_renderer->draw_user_meshes(cb, rp, ubo, false, rvd);
	geo_renderer->draw_objects_transparent(cb, rp, ubo, rvd);

	geo_renderer->draw_particles(cb, rp, rvd);
	geo_renderer->draw_user_meshes(cb, rp, ubo, true, rvd);

	gpu_timestamp_end(cb, ch_draw);
	PerformanceMonitor::end(ch_draw);
}

void WorldRendererVulkanForward::render_into_texture(FrameBuffer *fb, Camera *cam, RenderViewDataVK &rvd, const RenderParams& params) {
	auto cb = params.command_buffer;
	auto rp = params.render_pass;
	rp->clear_color[0] = world.background;

	cb->begin_render_pass(rp, fb);
	cb->set_viewport(rect(0, fb->width, 0, fb->height));

	std::swap(scene_view.cam, cam);
	scene_view.cam->update_matrices(params.desired_aspect_ratio); // argh, need more UBOs
	//prepare_lights(scene_view.cam, );
	auto sub_params = params.with_target(fb);
	draw(sub_params);
	std::swap(scene_view.cam, cam);

	cb->end_render_pass();
}

#endif

