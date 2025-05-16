/*
 * Renderer.h
 *
 *  Created on: Jan 4, 2020
 *      Author: michi
 */

#pragma once


class RenderTask;
struct rect;

#include "../graphics-fwd.h"
#include "../lib/base/pointer.h"
#include "../lib/base/optional.h"
#include "../lib/math/rect.h"

struct color;
class ResourceManager;

rect dynamicly_scaled_area(FrameBuffer *fb);
rect dynamicly_scaled_source();

struct RenderParams {
	float desired_aspect_ratio = 1;
	bool target_is_window = false;
	FrameBuffer* frame_buffer = nullptr;
	rect area;
#ifdef USING_VULKAN
	RenderPass *render_pass;
	CommandBuffer *command_buffer;
#endif

	RenderParams with_target(FrameBuffer *fb) const;
	RenderParams with_area(const rect& area) const;
	static const RenderParams WHATEVER;
	static RenderParams into_window(FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio = base::None);
	static RenderParams into_texture(FrameBuffer *frame_buffer, const base::optional<float>& aspect_ratio = base::None);
};

class Renderer : public VirtualBase {
public:
	explicit Renderer(const string& name);
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
	Context* context;
	ResourceManager* resource_manager;
};

class RenderTask : public  VirtualBase {
public:
	explicit RenderTask(const string& name);
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
	Context* context;
	ResourceManager* resource_manager;
};
