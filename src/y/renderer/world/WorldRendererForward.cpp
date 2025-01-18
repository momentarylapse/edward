/*
 * WorldRendererForward.cpp
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#include "WorldRendererForward.h"
#include "pass/ShadowRenderer.h"
#include "geometry/GeometryRenderer.h"
#include "../base.h"
#include "../helper/jitter.h"
#include "../helper/CubeMapSource.h"
#include "../path/RenderPath.h"
#include <lib/nix/nix.h>
#include <lib/image/image.h>
#include <lib/os/msg.h>
#include <y/ComponentManager.h>
#include <graphics-impl.h>

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


WorldRendererForward::WorldRendererForward(SceneView& scene_view) : WorldRenderer("world", scene_view) {
	resource_manager->load_shader_module("forward/module-surface.shader");

	geo_renderer = new GeometryRenderer(RenderPathType::Forward, scene_view);
	geo_renderer->set(GeometryRenderer::Flags::ALLOW_SKYBOXES | GeometryRenderer::Flags::ALLOW_CLEAR_COLOR | GeometryRenderer::Flags::ALLOW_OPAQUE | GeometryRenderer::Flags::ALLOW_TRANSPARENT);
	add_child(geo_renderer.get());
}

void WorldRendererForward::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	scene_view.cam->update_matrices(params.desired_aspect_ratio);

	geo_renderer->cur_rvd.set_projection_matrix(scene_view.cam->m_projection);
	geo_renderer->cur_rvd.set_view_matrix(scene_view.cam->m_view);
	geo_renderer->cur_rvd.update_lights();

	geo_renderer->prepare(params);

	PerformanceMonitor::end(ch_prepare);
}

void WorldRendererForward::draw(const RenderParams& params) {
	draw_with(params);
}
void WorldRendererForward::draw_with(const RenderParams& params) {

	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);

	geo_renderer->cur_rvd.prepare_scene(&scene_view);
	geo_renderer->draw(params);


	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}

#warning "TODO"
#if 0
void WorldRendererGLForward::render_into_texture(FrameBuffer *fb, Camera *cam, RenderViewData &rvd) {
	nix::bind_frame_buffer(fb);

	std::swap(scene_view.cam, cam);
	prepare_lights(cam, rvd);
	draw(RenderParams::into_texture(fb, 1.0f));
	std::swap(scene_view.cam, cam);
}
#endif

