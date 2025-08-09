/*
 * RenderPathDeferred.h
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#pragma once

#include "RenderPath.h"
#include <lib/yrenderer/scene/RenderViewData.h>

namespace yrenderer {
class SceneRenderer;
class ThroughShaderRenderer;
class TextureRenderer;

class RenderPathDeferred : public RenderPath {
public:
	shared_array<ygfx::Texture> gbuffer_textures;
	ygfx::UniformBuffer *ssao_sample_buffer;
	int ch_gbuf_out = -1;
	int ch_trans = -1;

	owned<TextureRenderer> gbuffer_renderer;

	owned<SceneRenderer> scene_renderer;
	owned<SceneRenderer> scene_renderer_background;
	owned<SceneRenderer> scene_renderer_trans;
	owned<ThroughShaderRenderer> out_renderer;

	RenderPathDeferred(Context* ctx, int width, int height, int shadow_resolution);

	void add_background_emitter(shared<MeshEmitter> emitter) override;
	void add_opaque_emitter(shared<MeshEmitter> emitter) override;
	void add_transparent_emitter(shared<MeshEmitter> emitter) override;

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;


	void render_out_from_gbuffer(ygfx::FrameBuffer *source, const RenderParams& params);
};

}
