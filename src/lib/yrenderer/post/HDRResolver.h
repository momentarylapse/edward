/*
 * HDRResolver.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>
#include <lib/math/vec2.h>

struct vec2;

namespace yrenderer {

class ComputeTask;
class TextureRenderer;
class ThroughShaderRenderer;

class HDRResolver : public Renderer {
public:
	HDRResolver(Context* ctx);
	~HDRResolver() override;

	bool user_resolution = false;
	vec2 resolution_scale = {1,1};
	int current_width, current_height;
	void set_resolution(int w, int h);
	void _set_resolution(int w, int h);
	int max_width, max_height;
	//void _set_max_resolution(int w, int h);
	void _create_textures(int width, int height);
	void _update_textures();

	void prepare(const RenderParams& params) override;

	float exposure = 1.0f;
	float bloom_factor = 1.0f;

	shared<ygfx::Texture> texture;
	shared<ygfx::DepthBuffer> depth_buffer;

	owned<ThroughShaderRenderer> out_renderer;

	static constexpr int MAX_BLOOM_LEVELS = 4;

	struct BloomLevel {
		shared<ygfx::Texture> tex_temp, depth_temp;
		shared<ygfx::Texture> tex_out, depth_out;
		owned<TextureRenderer> renderer[2];
		owned<ThroughShaderRenderer> tsr[2];
	} bloom_levels[MAX_BLOOM_LEVELS];

	owned<TextureRenderer> texture_renderer;

	static string magfilter;
};

}

