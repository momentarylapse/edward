#if HAS_LIB_GL

#include "Context.h"
#include "Window.h"
#include "Painter.h"
#include "../os/msg.h"
#include "../image/image.h"
#include <lib/nix/nix.h>
#include <lib/ygraphics/Context.h>

namespace nix {
	extern bool allow_separate_vertex_arrays;
	mat4 create_pixel_projection_matrix();
}

namespace xhui {

Context::Context(Window* w, ygfx::Context* ctx) {
	window = w;
	context = ctx;
}

Painter* Context::prepare_draw() {
	glfwMakeContextCurrent(window->window);
	int width, height;
	glfwGetWindowSize(window->window, &width, &height);
	int ww, hh;
	glfwGetFramebufferSize(window->window, &ww, &hh);

	if (!aux)
		aux = context->_create_auxiliary_stuff();

	const rect area = {0, (float)width, 0, (float)height};
	const rect native_area = {0, (float)ww, 0, (float)hh};
	return new Painter(this, window, native_area, area);
}

void Context::begin_draw(Painter *p) {
	// in case the event_id::JustBeforeDraw triggers off-screen rendering...
	nix::bind_frame_buffer(context->ctx->default_framebuffer);

	nix::start_frame_glfw(context->ctx, window->window);
	nix::set_projection_matrix(nix::create_pixel_projection_matrix() * mat4::translation({0,0,0.5f}) * mat4::scale(window->ui_scale, window->ui_scale, 1));
	//nix::clear(color(1, 0.15f, 0.15f, 0.3f));
	nix::set_cull(nix::CullMode::NONE);
	nix::set_z(false, false);
}

void Context::end_draw(Painter *p) {
	nix::end_frame_glfw();
	aux->reset_frame();
	iterate_text_caches();
}



Context* Context::create(Window* window) {
	glfwMakeContextCurrent(window->window);
	nix::allow_separate_vertex_arrays = true;
	nix::default_shader_bindings = false;
	auto ctx = new Context(window, new ygfx::Context(nix::init()));

	ctx->context->_create_default_textures();
	ctx->tex_white = ctx->context->tex_white;
	ctx->tex_black = ctx->context->tex_black;

	ctx->context->_create_auxiliary_stuff();


	return ctx;
}


void Context::rebuild_default_stuff() {
}

void Context::resize(int w, int h) {
	rebuild_default_stuff();
}

}

#endif
