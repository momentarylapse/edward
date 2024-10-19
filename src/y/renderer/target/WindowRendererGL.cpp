/*
 * WindowRendererGL.cpp
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */


#include "WindowRendererGL.h"
#ifdef USING_OPENGL
#if HAS_LIB_GLFW
#include <GLFW/glfw3.h>
#endif
#include "../base.h"
#include <lib/nix/nix.h>
#include "../../helper/PerformanceMonitor.h"

WindowRendererGL::WindowRendererGL(GLFWwindow* win) : TargetRenderer("win") {
	window = win;
#if HAS_LIB_GLFW
	glfwMakeContextCurrent(window);
	//glfwGetFramebufferSize(window, &width, &height);
#endif

	_frame_buffer = context->default_framebuffer;
}


bool WindowRendererGL::start_frame() {
#if HAS_LIB_GLFW
	nix::start_frame_glfw(context, window);
	//jitter_iterate();
	return true;
#else
	return false;
#endif
}

void WindowRendererGL::end_frame() {
#if HAS_LIB_GLFW
	PerformanceMonitor::begin(ch_end);
	gpu_timestamp_begin(ch_end);
	nix::end_frame_glfw();
	gpu_timestamp_end(ch_end);
	PerformanceMonitor::end(ch_end);
#endif
}


RenderParams WindowRendererGL::create_params(float aspect_ratio) {
	return RenderParams::into_window(_frame_buffer, aspect_ratio);
}

void WindowRendererGL::prepare(const RenderParams& params) {

}

void WindowRendererGL::draw(const RenderParams& params) {
	PerformanceMonitor::begin(ch_draw);
	auto sub_params = RenderParams::into_window(_frame_buffer, params.desired_aspect_ratio);
	for (auto c: children)
		c->prepare(sub_params);

	bool prev_srgb = nix::get_srgb();
	nix::set_srgb(true);
	nix::bind_frame_buffer(_frame_buffer);

	for (auto c: children)
		c->draw(sub_params);

	nix::set_srgb(prev_srgb);
	PerformanceMonitor::end(ch_draw);
}

#endif
