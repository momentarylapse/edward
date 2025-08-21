/*
 * Renderer.h
 *
 *  Created on: Jan 4, 2020
 *      Author: michi
 */

#pragma once
#include "ShaderManager.h"


struct rect;

#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/pointer.h>
#include <lib/base/optional.h>
#include <lib/math/rect.h>

struct color;

namespace yrenderer {

struct Context;
class RenderTask;
class ShaderManager;

rect dynamicly_scaled_area(ygfx::FrameBuffer *fb);
rect dynamicly_scaled_source();

struct RenderParams {
	float desired_aspect_ratio = 1;
	bool target_is_window = false;
	ygfx::FrameBuffer* frame_buffer = nullptr;
	rect area;
#ifdef USING_VULKAN
	ygfx::RenderPass *render_pass;
	ygfx::CommandBuffer *command_buffer;
#endif

	RenderParams with_target(ygfx::FrameBuffer *fb) const;
	RenderParams with_area(const rect& area) const;
	static const RenderParams WHATEVER;
	static RenderParams into_window(ygfx::FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio = base::None);
	static RenderParams into_texture(ygfx::FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio = base::None);
};

class Renderer : public VirtualBase {
public:
	explicit Renderer(Context* ctx, const string& name);
	~Renderer() override;

	Array<Renderer*> children;
	void add_child(Renderer *child);
	Array<RenderTask*> sub_tasks;
	void add_sub_task(RenderTask* child);

	// (vulkan: BEFORE/OUTSIDE a render pass)
	// can render into separate targets
	virtual void prepare(const RenderParams& params);

	// assume, parent has already bound the frame buffer
	// (vulkan: INSIDE an already started render pass)
	// just draw into that
	virtual void draw(const RenderParams& params);


	int channel;
	int ch_prepare;
	Context* ctx;
	ShaderManager* shader_manager;
};

class RenderTask : public  VirtualBase {
public:
	explicit RenderTask(Context* ctx, const string& name);
	~RenderTask() override;

	Array<Renderer*> children;
	void add_child(Renderer *child);
	Array<RenderTask*> sub_tasks;
	void add_sub_task(RenderTask* child);

	void prepare_children(const RenderParams& params);

	virtual void render(const RenderParams& params) = 0;

	int channel;
	bool active = true;
	int _priority = 0;
	Context* ctx;
	ShaderManager* shader_manager;
};

}
