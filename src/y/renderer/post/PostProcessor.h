/*
 * PostProcessor.h
 *
 *  Created on: Dec 13, 2021
 *      Author: michi
 */


#pragma once

#include "../Renderer.h"
#include "../../graphics-fwd.h"
#include <lib/base/callable.h>

struct vec2;
class Any;
class PostProcessor;

struct PostProcessorStage : public Renderer {
	PostProcessorStage(const string &name);
	PostProcessor *post = nullptr;

	VertexBuffer *vb_2d;
};

struct PostProcessorStageUser : public PostProcessorStage {
	using Callback = Callable<void(const RenderParams&)>;
	const Callback *func_prepare = nullptr;
	const Callback *func_draw = nullptr;

	PostProcessorStageUser(const Callback *p, const Callback *d);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;
};

class PostProcessor : public Renderer {
public:
	PostProcessor();
	virtual ~PostProcessor();

	Array<PostProcessorStage*> stages;
	void add_stage(const PostProcessorStageUser::Callback *p, const PostProcessorStageUser::Callback *d);
	void reset();
	void rebuild();

	PostProcessorStage *hdr = nullptr;
	void set_hdr(PostProcessorStage *hdr);

	/*void process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader, const Any &data);
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

	FrameBuffer *frame_buffer() const override;
	DepthBuffer *depth_buffer() const override;
	bool forwarding_into_window() const override;

	DepthBuffer *_depth_buffer = nullptr;
	shared<Shader> shader_blur;
	shared<Shader> shader_out;

	VertexBuffer *vb_2d;

	int ch_post_blur = -1, ch_out = -1;*/
};

