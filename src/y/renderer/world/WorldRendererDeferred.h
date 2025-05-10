/*
 * WorldRendererDeferred.h
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#pragma once

#include "WorldRenderer.h"
#include "../scene/RenderViewData.h"

class SceneRenderer;
class ThroughShaderRenderer;
class TextureRenderer;

class WorldRendererDeferred : public WorldRenderer {
public:
	shared_array<Texture> gbuffer_textures;
	UniformBuffer *ssao_sample_buffer;
	int ch_gbuf_out = -1;
	int ch_trans = -1;

	owned<TextureRenderer> gbuffer_renderer;

	owned<SceneRenderer> scene_renderer;
	owned<SceneRenderer> scene_renderer_background;
	owned<SceneRenderer> scene_renderer_trans;
	owned<ThroughShaderRenderer> out_renderer;

	WorldRendererDeferred(SceneView& scene_view, int width, int height);
	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;


	void render_out_from_gbuffer(FrameBuffer *source, const RenderParams& params);
};
