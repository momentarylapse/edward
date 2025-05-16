/*
 * HDRResolver.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include "../Renderer.h"

struct vec2;
class Camera;
class ComputeTask;
class TextureRenderer;
class ThroughShaderRenderer;

class HDRResolver : public Renderer {
public:
	HDRResolver(Camera *cam, const shared<Texture>& tex, const shared<DepthBuffer>& depth_buffer);
	~HDRResolver() override;

	void prepare(const RenderParams& params) override;

	Camera *cam;

	shared<Texture> tex_main;

	owned<ThroughShaderRenderer> out_renderer;

	static constexpr int MAX_BLOOM_LEVELS = 4;

	struct BloomLevel {
		shared<Texture> tex_temp;
		shared<Texture> tex_out;
		owned<TextureRenderer> renderer[2];
		owned<ThroughShaderRenderer> tsr[2];
	} bloom_levels[MAX_BLOOM_LEVELS];

	shared<DepthBuffer> _depth_buffer;
};

