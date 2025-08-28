/*
 * RenderPathForward.cpp
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#include "RenderPathForward.h"
#include <lib/yrenderer/scene/pass/ShadowRenderer.h>
#include <lib/yrenderer/scene/pass/CubeMapRenderer.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/CubeMapSource.h>
#include <lib/image/image.h>
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/ShaderManager.h>


namespace yrenderer {

RenderPathForward::RenderPathForward(Context* ctx, int shadow_resolution) : RenderPath(ctx, "fwd") {
	if (ctx) {
		shader_manager->load_shader_module("forward/module-surface.shader");

		scene_renderer = new SceneRenderer(ctx, RenderPathType::Forward, scene_view);

		create_shadow_renderer(shadow_resolution);
		create_cube_renderer();
	}
}

void RenderPathForward::remove_all_emitters() {
	scene_renderer->emitters.clear();
	shadow_renderer->remove_all_emitters();
	cube_map_renderer->remove_all_emitters();
}

void RenderPathForward::add_background_emitter(shared<MeshEmitter> emitter) {
	scene_renderer->add_emitter(emitter);
	cube_map_renderer->add_emitter(emitter);
}

void RenderPathForward::add_opaque_emitter(shared<MeshEmitter> emitter) {
	scene_renderer->add_emitter(emitter);
	shadow_renderer->add_emitter(emitter);
	cube_map_renderer->add_emitter(emitter);
}

void RenderPathForward::add_transparent_emitter(shared<MeshEmitter> emitter) {
	scene_renderer->add_emitter(emitter);
	//cube_map_renderer->add_emitter(emitter);
}




void RenderPathForward::prepare(const RenderParams& params) {
	profiler::begin(ch_prepare);

	scene_renderer->set_view(params, view);
	scene_renderer->background_color = background_color;
	scene_renderer->prepare(params);

	shadow_renderer->render(params);

	profiler::end(ch_prepare);
}

void RenderPathForward::draw(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	scene_renderer->draw(params);

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

}
