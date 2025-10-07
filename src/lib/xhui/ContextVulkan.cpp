#if HAS_LIB_VULKAN

#include "Context.h"
#include "Window.h"
#include "Painter.h"
#include "../os/msg.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>

namespace xhui {

Context::Context(Window* w, ygfx::Context* ctx) {
	window = w;
	context = ctx;
	device = context->device;
}

Painter* Context::prepare_draw() {
	context->make_current();
	if (!swap_chain->acquire_image(&image_index, image_available_semaphore)) {
		rebuild_default_stuff();
		return nullptr;
	}

	auto f = wait_for_frame_fences[image_index];
	f->wait();
	f->reset();

	if (!aux) {
		aux = context->_create_auxiliary_stuff();
	}

	auto cb = current_command_buffer();
	aux->cb = cb;
	cb->begin();

	int width = (int)((float)swap_chain->width / window->ui_scale);
	int height = (int)((float)swap_chain->height / window->ui_scale);
	const rect area = {0, (float)width, 0, (float)height};
	const rect native_area = {0, (float)swap_chain->width, 0, (float)swap_chain->height};

	auto p = new Painter(this, window, native_area, area);
	p->cb = cb;
	return p;
}

void Context::begin_draw(Painter* p) {
	auto cb = current_command_buffer();
	auto fb = current_frame_buffer();

	cb->set_viewport(p->native_area);
	cb->begin_render_pass(render_pass, fb);
	cb->set_scissor(p->native_area);
	cb->clear(p->native_area, {Black}, 1);
}

void Context::end_draw(Painter *p) {
	auto cb = current_command_buffer();
	cb->end_render_pass();
	cb->end();


	auto f = wait_for_frame_fences[image_index];
	context->device->present_queue.submit(cb, {image_available_semaphore}, {render_finished_semaphore}, f);

	swap_chain->present(image_index, {render_finished_semaphore});
	device->wait_idle();

	aux->reset_frame();
	iterate_text_caches();
	delete p;
}



void Context::_create_swap_chain_and_stuff() {
	bool gamma_correction = (color_space_shaders == ColorSpace::Linear) and (color_space_display == ColorSpace::SRGB);
	if (swap_chain) {
		int w, h;
		glfwGetFramebufferSize(window->window, &w, &h);
		swap_chain->rebuild(w, h, gamma_correction);
	} else {
		swap_chain = vulkan::SwapChain::create_for_glfw(device, window->window, gamma_correction);
	}
	auto swap_images = swap_chain->create_textures();
	for (auto t: swap_images)
		wait_for_frame_fences.add(new vulkan::Fence(device));

	for (auto t: swap_images)
		command_buffers.add(device->command_pool->create_command_buffer());

	depth_buffer = swap_chain->create_depth_buffer();
	render_pass = swap_chain->create_render_pass(depth_buffer);
	frame_buffers = swap_chain->create_frame_buffers(render_pass, depth_buffer);

	aux = context->_create_auxiliary_stuff();
	aux->rebuild(render_pass);
}

Context* Context::create(Window* window) {

	static vulkan::Instance* global_instance = nullptr;

	if (!global_instance) {
		global_instance = vulkan::init({"glfw", "validation", "api=1.3", "verbosity=1"});
	}
	auto instance = global_instance;
	auto surface = instance->create_glfw_surface(window->window);
	auto device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation"});
	auto ctx = new Context(window, new ygfx::Context(instance, device));
	//msg_write("device found");

	ctx->context->_create_default_textures();
	ctx->tex_white = ctx->context->tex_white;
	ctx->tex_black = ctx->context->tex_black;

	ctx->context->_create_auxiliary_stuff();


	ctx->image_available_semaphore = new vulkan::Semaphore(device);
	ctx->render_finished_semaphore = new vulkan::Semaphore(device);


	ctx->framebuffer_resized = false;



	ctx->_create_swap_chain_and_stuff();
	return ctx;
}


void Context::rebuild_default_stuff() {
	//msg_write("recreate swap chain");

	device->wait_idle();

	//_delete_swap_chain_and_stuff();
	_create_swap_chain_and_stuff();
}

void Context::resize(int w, int h) {
	rebuild_default_stuff();
}

vulkan::CommandBuffer* Context::current_command_buffer() const {
	return command_buffers[image_index];
}

vulkan::FrameBuffer* Context::current_frame_buffer() const {
	return frame_buffers[image_index];
}



}

#endif
