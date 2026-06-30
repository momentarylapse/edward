#if HAS_LIB_GL

#include "Context.h"
#include "Window.h"
#include "Painter.h"
#include <lib/image/image.h>
#include <lib/nix/nix.h>
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/TextCache.h>

namespace nix {
	extern bool allow_separate_vertex_arrays;
}

namespace xhui {

Context::Context(Window* w, ygfx::Context* ctx, ygfx::FontManager* fm) {
	window = w;
	context = ctx;
	font_manager = fm;
}

Painter* Context::prepare_draw() {
	glfwMakeContextCurrent(window->window);
	int width, height;
	glfwGetWindowSize(window->window, &width, &height);
	int ww, hh;
	glfwGetFramebufferSize(window->window, &ww, &hh);

	const rect area = {0, (float)width, 0, (float)height};
	const rect native_area = {0, (float)ww, 0, (float)hh};
	//if (!painter)
	painter = new Painter(this, window, native_area, area);
	painter->text_cache = text_cache.get();
	return painter.get();
}

void Context::begin_draw(Painter *p) {
	bool gamma_correction = (color_space_display == ColorSpace::SRGB) and (color_space_shaders == ColorSpace::Linear);
	nix::set_srgb(gamma_correction);

	// in case the event_id::JustBeforeDraw triggers off-screen rendering...
	nix::bind_frame_buffer(context->ctx->default_framebuffer);

	nix::start_frame_glfw(context->ctx, window->window);
	p->prepare_2d_drawing();
	//nix::clear(color(1, 0.15f, 0.15f, 0.3f));
}

void Context::end_draw(Painter *p) {
	nix::end_frame_glfw();
	nix::set_srgb(false);
	aux->reset_frame();
	text_cache->iterate();
}



Context* Context::create(Window* window) {
	glfwMakeContextCurrent(window->window);
	nix::allow_separate_vertex_arrays = true;
	nix::default_shader_bindings = false;
	auto ctx = new Context(window, new ygfx::Context(nix::init()), global_font_manager);

	ctx->context->color_space_shaders = color_space_shaders;
	ctx->context->color_space_input = color_space_input;

	ctx->context->_create_default_textures();
	ctx->tex_white = ctx->context->tex_white;

	ctx->aux = ctx->context->_create_auxiliary_stuff();

	ctx->text_cache = new ygfx::TextCache(ctx->aux);
	global_text_cache = ctx->text_cache.get();

	return ctx;
}


void Context::rebuild_default_stuff() {
}

void Context::resize(int w, int h) {
	rebuild_default_stuff();
}

}

#endif
