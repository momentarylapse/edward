/*
 * HDRResolver.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>

struct vec2;

namespace yrenderer {

class ComputeTask;
class TextureRenderer;
class ThroughShaderRenderer;

class HDRResolver : public Renderer {
public:
	HDRResolver(Context* ctx, const shared<ygfx::Texture>& tex, const shared<ygfx::DepthBuffer>& depth_buffer, bool manual_mode = false);
	~HDRResolver() override;

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	float exposure = 1.0f;
	float bloom_factor = 1.0f;

	shared<ygfx::Texture> tex_main;

	owned<ThroughShaderRenderer> out_renderer;

	static constexpr int MAX_BLOOM_LEVELS = 4;

	struct BloomLevel {
		shared<ygfx::Texture> tex_temp;
		shared<ygfx::Texture> tex_out;
		owned<TextureRenderer> renderer[2];
		owned<ThroughShaderRenderer> tsr[2];
	} bloom_levels[MAX_BLOOM_LEVELS];

	owned<TextureRenderer> texture_renderer;

	shared<ygfx::DepthBuffer> _depth_buffer;
};

}

