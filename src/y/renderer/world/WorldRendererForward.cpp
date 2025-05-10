/*
 * WorldRendererForward.cpp
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#include "WorldRendererForward.h"
#include "../scene/pass/ShadowRenderer.h"
#include "../base.h"
#include "../helper/CubeMapSource.h"
#include "../path/RenderPath.h"
#include <lib/image/image.h>
#include <renderer/world/emitter/WorldInstancedEmitter.h>
#include <renderer/world/emitter/WorldModelsEmitter.h>
#include <renderer/world/emitter/WorldParticlesEmitter.h>
#include <renderer/world/emitter/WorldSkyboxEmitter.h>
#include <renderer/world/emitter/WorldTerrainsEmitter.h>
#include <renderer/world/emitter/WorldUserMeshesEmitter.h>
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../world/Camera.h"
#include "../../world/World.h"
#include <graphics-impl.h>


WorldRendererForward::WorldRendererForward(SceneView& scene_view) : WorldRenderer("world", scene_view) {
	resource_manager->load_shader_module("forward/module-surface.shader");

	scene_renderer = new SceneRenderer(RenderPathType::Forward, scene_view);
	scene_renderer->add_emitter(new WorldSkyboxEmitter);
	scene_renderer->add_emitter(new WorldModelsEmitter);
	scene_renderer->add_emitter(new WorldTerrainsEmitter);
	scene_renderer->add_emitter(new WorldUserMeshesEmitter);
	scene_renderer->add_emitter(new WorldInstancedEmitter);
	scene_renderer->add_emitter(new WorldParticlesEmitter);
}

void WorldRendererForward::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	scene_view.cam->update_matrix_cache(params.desired_aspect_ratio);
	
	scene_renderer->set_view_from_camera(params, scene_view.cam);
	scene_renderer->prepare(params);

	PerformanceMonitor::end(ch_prepare);
}

void WorldRendererForward::draw(const RenderParams& params) {
	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);

	scene_renderer->draw(params);

	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}

