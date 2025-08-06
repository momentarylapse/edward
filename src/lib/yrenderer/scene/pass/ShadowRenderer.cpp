/*
 * ShadowRenderer.cpp
 *
 *  Created on: Dec 11, 2022
 *      Author: michi
 */

#include "ShadowRenderer.h"
#include "../Light.h"
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/Material.h>
#include "lib/yrenderer/scene/CameraParams.h"
#include <lib/yrenderer/target/TextureRenderer.h>

namespace yrenderer {

ShadowRenderer::Cascade::Cascade() = default;
ShadowRenderer::Cascade::~Cascade() = default;

ShadowRenderer::ShadowRenderer(Context* ctx, SceneView* parent, shared_array<MeshEmitter> emitters, int resolution) :
		RenderTask(ctx, "shdw")
{

	material = new Material(ctx);
	material->pass0.shader_path = "shadow.shader";

	parent_scene = parent;
	scene_view.shadow_indices.clear();

	for (int i=0; i<NUM_CASCADES; i++) {
		auto& c = cascades[i];
		c.scene_renderer = new SceneRenderer(ctx, RenderPathType::Forward, scene_view);
		c.scene_renderer->is_shadow_pass = true;
		c.scene_renderer->rvd.material_shadow = material.get();
		for (auto e: weak(emitters))
			c.scene_renderer->add_emitter(e);

		shared tex = new ygfx::Texture(resolution, resolution, "rgba:i8");
		c.depth_buffer = new ygfx::DepthBuffer(resolution, resolution, "d:f32");
		c.texture_renderer = new TextureRenderer(ctx, format("cas%d", i), {tex, c.depth_buffer}, {"autoclear"});
		c.scale = (i == 0) ? 4.0f : 1.0f;
		c.texture_renderer->add_child(c.scene_renderer.get());
	}
}

void ShadowRenderer::set_projection(const mat4& proj) {
	for (int i=0; i<NUM_CASCADES; i++) {
		auto& c = cascades[i];

		// shader for vulkan is currently flipping y!
/*#ifdef USING_OPENGL
		auto m = mat4::scale(c.scale, c.scale, 1);
#else*/
		auto m = mat4::scale(c.scale, -c.scale, 1);
//#endif
		const mat4 mp = m * proj;
		c.scene_renderer->set_view(RenderParams::WHATEVER, CameraParams{vec3::ZERO, quaternion::ID}, &mp);
	}
}

void ShadowRenderer::render(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	for (int index: parent_scene->shadow_indices) {
		set_projection(parent_scene->lights[index]->shadow_projection);

		for (int i=0; i<NUM_CASCADES; i++)
			render_cascade(params, cascades[i]);
	}

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

void ShadowRenderer::render_cascade(const RenderParams& _params, Cascade& c) {
	auto params = _params.with_target(c.texture_renderer->frame_buffer.get());
	params.desired_aspect_ratio = 1.0f;

	// all opaque meshes
	c.texture_renderer->render(params);
}

}
