/*
 * Renderer.h
 *
 *  Created on: Jan 4, 2020
 *      Author: michi
 */

#pragma once


class rect;

#include "../graphics-fwd.h"
#include "../lib/base/pointer.h"

class rect;
class color;
class ResourceManager;

rect dynamicly_scaled_area(FrameBuffer *fb);
rect dynamicly_scaled_source();

struct RenderParams {
	float desired_aspect_ratio;
	bool target_is_window;
	FrameBuffer *frame_buffer;
#ifdef USING_VULKAN
	RenderPass *render_pass;
	CommandBuffer *command_buffer;
#endif

	RenderParams with_target(FrameBuffer *fb) const;
	static const RenderParams WHATEVER;
	static RenderParams into_window(FrameBuffer *frame_buffer, float aspect_ratio);
	static RenderParams into_texture(FrameBuffer *frame_buffer, float aspect_ratio);
};

class Renderer {
public:
	Renderer(const string &name);
	virtual ~Renderer();

	Array<Renderer*> children;
	void add_child(Renderer *child);

	// (vulkan: BEFORE/OUTSIDE a render pass)
	// can render into separate targets
	virtual void prepare(const RenderParams& params);

	// assume, parent has already bound the frame buffer
	// (vulkan: INSIDE an already started render pass)
	// just draw into that
	virtual void draw(const RenderParams& params) = 0;


	int channel;
	int ch_draw;
	int ch_prepare;
	Context *context;
	ResourceManager *resource_manager;
};
