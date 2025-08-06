/*
 * PostProcessorVulkan.h
 *
 *  Created on: Dec 7, 2021
 *      Author: michi
 */

#pragma once

#include "PostProcessor.h"
#ifdef USING_VULKAN
#include <lib/ygraphics/graphics-fwd.h>

struct vec2;
class Any;

class PostProcessorVulkan : public PostProcessor {
public:
	PostProcessorVulkan(yrenderer::Context* ctx);
	virtual ~PostProcessorVulkan();

	void prepare(const yrenderer::RenderParams& params) override;
	void draw(const yrenderer::RenderParams& params) override;

	void process(const Array<ygfx::Texture*> &source, ygfx::FrameBuffer *target, ygfx::Shader *shader, const Any &data);
	ygfx::FrameBuffer* do_post_processing(ygfx::FrameBuffer *source);

	shared<ygfx::Shader> shader_depth;
	shared<ygfx::Shader> shader_resolve_multisample;
	void process_blur(ygfx::FrameBuffer *source, ygfx::FrameBuffer *target, float threshold, const vec2 &axis);
	void process_depth(ygfx::FrameBuffer *source, ygfx::FrameBuffer *target, const vec2 &axis);
	//void process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader);
	//FrameBuffer* do_post_processing(ygfx::FrameBuffer *source);
	ygfx::FrameBuffer* resolve_multisampling(ygfx::FrameBuffer *source);

	shared<ygfx::FrameBuffer> fb1;
	shared<ygfx::FrameBuffer> fb2;
	ygfx::FrameBuffer *next_fb(ygfx::FrameBuffer *cur);

	ygfx::DepthBuffer *_depth_buffer = nullptr;
	shared<ygfx::Shader> shader_blur;
	shared<ygfx::Shader> shader_out;

	ygfx::VertexBuffer *vb_2d;

	int ch_post_blur = -1, ch_out = -1;
};

#endif
