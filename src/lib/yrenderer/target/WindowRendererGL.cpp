/*
 * WindowRendererGL.cpp
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */


#include "WindowRenderer.h"
#ifdef USING_OPENGL
#if HAS_LIB_GLFW
#include <GLFW/glfw3.h>
#endif
#include <lib/yrenderer/Context.h>
#include <lib/nix/nix.h>
#include <lib/profiler/Profiler.h>
#include <lib/ygraphics/Context.h>

namespace yrenderer {

WindowRenderer::WindowRenderer(Context* ctx, GLFWwindow* win, bool _gamma_correction) : TargetRenderer(ctx, "win") {
	window = win;
	gamma_correction = _gamma_correction;
	if (ctx and window) {
#if HAS_LIB_GLFW
		glfwMakeContextCurrent(window);
		//glfwGetFramebufferSize(window, &width, &height);
#endif

		_frame_buffer = ctx->context->ctx->default_framebuffer;
	}
}


bool WindowRenderer::start_frame() {
#if HAS_LIB_GLFW
	nix::start_frame_glfw(ctx->context->ctx, window);
	//jitter_iterate();
	return true;
#else
	return false;
#endif
}

void WindowRenderer::end_frame(const RenderParams& params) {
#if HAS_LIB_GLFW
	profiler::begin(ch_end);
	ctx->gpu_timestamp_begin(params, ch_end);
	nix::end_frame_glfw();
	ctx->gpu_timestamp_end(params, ch_end);
	profiler::end(ch_end);
#endif
}


RenderParams WindowRenderer::create_params(float aspect_ratio) {
	return RenderParams::into_window(_frame_buffer, aspect_ratio);
}

void WindowRenderer::prepare(const RenderParams& params) {

}

void WindowRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	auto sub_params = RenderParams::into_window(_frame_buffer, params.desired_aspect_ratio);
	for (auto c: children)
		c->prepare(sub_params);

	bool prev_srgb = nix::get_srgb();
	nix::set_srgb(gamma_correction);
	nix::bind_frame_buffer(_frame_buffer);

	for (auto c: children)
		c->draw(sub_params);

	nix::set_srgb(prev_srgb);
	profiler::end(channel);
}

}

#endif
