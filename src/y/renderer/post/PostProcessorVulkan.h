/*
 * PostProcessorVulkan.h
 *
 *  Created on: Dec 7, 2021
 *      Author: michi
 */

#pragma once

#include "PostProcessor.h"
#ifdef USING_VULKAN
#include "../../graphics-fwd.h"

class vec2;
class Any;

class PostProcessorVulkan : public PostProcessor {
public:
	PostProcessorVulkan();
	virtual ~PostProcessorVulkan();

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader, const Any &data);
	FrameBuffer* do_post_processing(FrameBuffer *source);

	shared<Shader> shader_depth;
	shared<Shader> shader_resolve_multisample;
	void process_blur(FrameBuffer *source, FrameBuffer *target, float threshold, const vec2 &axis);
	void process_depth(FrameBuffer *source, FrameBuffer *target, const vec2 &axis);
	//void process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader);
	//FrameBuffer* do_post_processing(FrameBuffer *source);
	FrameBuffer* resolve_multisampling(FrameBuffer *source);

	shared<FrameBuffer> fb1;
	shared<FrameBuffer> fb2;
	FrameBuffer *next_fb(FrameBuffer *cur);

	DepthBuffer *_depth_buffer = nullptr;
	shared<Shader> shader_blur;
	shared<Shader> shader_out;

	VertexBuffer *vb_2d;

	int ch_post_blur = -1, ch_out = -1;
};

#endif
