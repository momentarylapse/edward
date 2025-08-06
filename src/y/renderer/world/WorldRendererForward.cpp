/*
 * WorldRendererForward.cpp
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#include "WorldRendererForward.h"
#include <lib/yrenderer/scene/pass/ShadowRenderer.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/CubeMapSource.h>
#include "../path/RenderPath.h"
#include <lib/image/image.h>
#include <lib/profiler/Profiler.h>
#include <renderer/world/emitter/WorldInstancedEmitter.h>
#include <renderer/world/emitter/WorldModelsEmitter.h>
#include <renderer/world/emitter/WorldParticlesEmitter.h>
#include <renderer/world/emitter/WorldSkyboxEmitter.h>
#include <renderer/world/emitter/WorldTerrainsEmitter.h>
#include <renderer/world/emitter/WorldUserMeshesEmitter.h>
#include <lib/yrenderer/ShaderManager.h>
#include "../../world/Camera.h"


WorldRendererForward::WorldRendererForward(yrenderer::Context* ctx, Camera* cam, yrenderer::SceneView& scene_view) : WorldRenderer(ctx, "world", cam, scene_view) {
	shader_manager->load_shader_module("forward/module-surface.shader");

	scene_renderer = new yrenderer::SceneRenderer(ctx, yrenderer::RenderPathType::Forward, scene_view);
	scene_renderer->add_emitter(new WorldSkyboxEmitter(ctx));
	scene_renderer->add_emitter(new WorldModelsEmitter(ctx));
	scene_renderer->add_emitter(new WorldTerrainsEmitter(ctx));
	scene_renderer->add_emitter(new WorldUserMeshesEmitter(ctx));
	scene_renderer->add_emitter(new WorldInstancedEmitter(ctx));
	scene_renderer->add_emitter(new WorldParticlesEmitter(ctx, cam));
}

void WorldRendererForward::prepare(const yrenderer::RenderParams& params) {
	profiler::begin(ch_prepare);
	cam->update_matrix_cache(params.desired_aspect_ratio);
	
	scene_renderer->set_view(params, cam->params());
	scene_renderer->prepare(params);

	profiler::end(ch_prepare);
}

void WorldRendererForward::draw(const yrenderer::RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	scene_renderer->draw(params);

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

