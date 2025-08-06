/*
 * WindowRendererVulkan.cpp
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */

#include "WindowRenderer.h"
#ifdef USING_VULKAN
#include "../Context.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/profiler/Profiler.h>
#include <lib/os/msg.h>
#include <lib/math/rect.h>

namespace yrenderer {

SurfaceRendererVulkan::SurfaceRendererVulkan(Context* ctx, const string& name) : TargetRenderer(ctx, name) {
	if (ctx) {
		device = ctx->device;

		image_available_semaphore = new vulkan::Semaphore(device);
		render_finished_semaphore = new vulkan::Semaphore(device);
	}


	framebuffer_resized = false;
}

SurfaceRendererVulkan::~SurfaceRendererVulkan() = default;



void SurfaceRendererVulkan::_create_swap_chain_and_stuff() {
	create_swap_chain();
	swap_images = swap_chain->create_textures();
	for (auto t: swap_images)
		wait_for_frame_fences.add(new vulkan::Fence(device));

	for (auto t: swap_images)
		command_buffers.add(device->command_pool->create_command_buffer());

	depth_buffer = swap_chain->create_depth_buffer();
	default_render_pass = swap_chain->create_render_pass(depth_buffer);
	frame_buffers = swap_chain->create_frame_buffers(default_render_pass, depth_buffer);
}


/*func _delete_swap_chain_and_stuff()
	for fb in frame_buffers
		del fb
	del _default_render_pass
	del depth_buffer
	del swap_chain*/

void SurfaceRendererVulkan::rebuild_default_stuff() {
	msg_write("recreate swap chain");

	device->wait_idle();

	delete swap_chain;
	//_delete_swap_chain_and_stuff();
	_create_swap_chain_and_stuff();
}


bool SurfaceRendererVulkan::start_frame() {

	if (!swap_chain->acquire_image(&image_index, image_available_semaphore)) {
		rebuild_default_stuff();
		return false;
	}

	auto f = wait_for_frame_fences[image_index];
	f->wait();
	f->reset();

	command_buffers[image_index]->begin();

	return true;
}

void SurfaceRendererVulkan::end_frame(const RenderParams& params) {
	profiler::begin(ch_end);
	params.command_buffer->end();
	auto f = wait_for_frame_fences[image_index];
	device->present_queue.submit(command_buffers[image_index], {image_available_semaphore}, {render_finished_semaphore}, f);

	swap_chain->present(image_index, {render_finished_semaphore});

	device->wait_idle();
	profiler::end(ch_end);
}

RenderParams SurfaceRendererVulkan::create_params(float aspect_ratio) {
	auto p = RenderParams::into_window(frame_buffers[image_index], aspect_ratio);
	p.command_buffer = command_buffers[image_index];
	p.render_pass = default_render_pass;
	return p;
}

void SurfaceRendererVulkan::prepare(const RenderParams& params) {

}

void SurfaceRendererVulkan::draw(const RenderParams& params) {
	profiler::begin(channel);
	auto cb = params.command_buffer;
	auto rp = params.render_pass;
	auto fb = params.frame_buffer;

	//cb->begin();
	for (auto c: children)
		c->prepare(params);

	cb->set_viewport({0, (float)swap_chain->width, 0, (float)swap_chain->height});
	cb->begin_render_pass(rp, fb);

	for (auto c: children)
		c->draw(params);

	cb->end_render_pass();
	//cb->end();
	profiler::end(channel);
}


#ifdef HAS_LIB_GLFW
WindowRenderer::WindowRenderer(Context* ctx, GLFWwindow* _window) :
		SurfaceRendererVulkan(ctx, "win") {
	window = _window;
	if (ctx and window)
		_create_swap_chain_and_stuff();
}

void WindowRenderer::create_swap_chain() {
	swap_chain = vulkan::SwapChain::create_for_glfw(device, window);
}
#endif

HeadlessSurfaceRendererVulkan::HeadlessSurfaceRendererVulkan(Context* ctx, int _width, int _height) :
		SurfaceRendererVulkan(ctx, "headless") {
	width = _width;
	height = _height;
}

void HeadlessSurfaceRendererVulkan::create_swap_chain() {
	swap_chain = vulkan::SwapChain::create(device, width, height);
}

xfer<HeadlessSurfaceRendererVulkan> HeadlessSurfaceRendererVulkan::create(Context* ctx, int width, int height) {
	auto r = new HeadlessSurfaceRendererVulkan(ctx, width, height);
	r->_create_swap_chain_and_stuff();
	return r;
}

}

#endif
