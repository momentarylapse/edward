/*
 * PostProcessor.h
 *
 *  Created on: Dec 13, 2021
 *      Author: michi
 */


#pragma once

#include <lib/yrenderer/Renderer.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/callable.h>

struct vec2;
class Any;

namespace yrenderer {

class PostProcessor;

struct PostProcessorStage : public Renderer {
	PostProcessorStage(Context* ctx, const string &name);
	PostProcessor *post = nullptr;

	ygfx::VertexBuffer *vb_2d;
};

struct PostProcessorStageUser : PostProcessorStage {
	using Callback = Callable<void(const RenderParams&)>;
	const Callback *func_prepare = nullptr;
	const Callback *func_draw = nullptr;

	PostProcessorStageUser(Context* ctx, const string& name, const Callback *p, const Callback *d);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;
};

class PostProcessorBase : public Renderer {
public:
	explicit PostProcessorBase(Context* ctx);
	~PostProcessorBase() override;

	Array<PostProcessorStage*> stages;
	void add_stage(const string& name, const PostProcessorStageUser::Callback *p, const PostProcessorStageUser::Callback *d);
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


#ifdef USING_VULKAN

class PostProcessor : public PostProcessorBase {
public:
	PostProcessor(Context* ctx);
	virtual ~PostProcessor();

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

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

#else


class PostProcessor : public PostProcessorBase {
public:
	PostProcessor(Context* ctx, int width, int height);
	~PostProcessor() override;

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void process(const Array<ygfx::Texture*> &source, ygfx::FrameBuffer *target, ygfx::Shader *shader, const Any &data);
	ygfx::FrameBuffer* do_post_processing(ygfx::FrameBuffer *source);

	shared<ygfx::Shader> shader_depth;
	shared<ygfx::Shader> shader_resolve_multisample;
	void process_blur(ygfx::FrameBuffer *source, ygfx::FrameBuffer *target, float threshold, const vec2 &axis);
	void process_depth(ygfx::FrameBuffer *source, ygfx::FrameBuffer *target, const vec2 &axis);
	//void process(const Array<Texture*> &source, ygfx::FrameBuffer *target, ygfx::Shader *shader);
	//ygfx::FrameBuffer* do_post_processing(ygfx::FrameBuffer *source);
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

}

