/*
 * WorldRendererDeferred.h
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#pragma once

#include "WorldRenderer.h"
#include <lib/yrenderer/scene/RenderViewData.h>

namespace yrenderer {
	class SceneRenderer;
	class ThroughShaderRenderer;
	class TextureRenderer;
}

class WorldRendererDeferred : public WorldRenderer {
public:
	shared_array<ygfx::Texture> gbuffer_textures;
	ygfx::UniformBuffer *ssao_sample_buffer;
	int ch_gbuf_out = -1;
	int ch_trans = -1;

	owned<yrenderer::TextureRenderer> gbuffer_renderer;

	owned<yrenderer::SceneRenderer> scene_renderer;
	owned<yrenderer::SceneRenderer> scene_renderer_background;
	owned<yrenderer::SceneRenderer> scene_renderer_trans;
	owned<yrenderer::ThroughShaderRenderer> out_renderer;

	WorldRendererDeferred(yrenderer::Context* ctx, Camera* cam, yrenderer::SceneView& scene_view, int width, int height);
	void prepare(const yrenderer::RenderParams& params) override;
	void draw(const yrenderer::RenderParams& params) override;


	void render_out_from_gbuffer(ygfx::FrameBuffer *source, const yrenderer::RenderParams& params);
};
