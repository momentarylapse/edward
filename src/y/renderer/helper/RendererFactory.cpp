/*
 * RendererFactory.cpp
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#include "RendererFactory.h"
#include "../base.h"
#include <graphics-impl.h>
#include "../world/WorldRenderer.h"
#ifdef USING_VULKAN
	#include "../world/WorldRendererVulkan.h"
	#include "../world/WorldRendererVulkanForward.h"
	#include "../world/WorldRendererVulkanRayTracing.h"
	#include "../gui/GuiRendererVulkan.h"
	#include "../post/HDRRendererVulkan.h"
	#include "../post/PostProcessorVulkan.h"
	#include "../regions/RegionRendererVulkan.h"
	#include "../target/WindowRendererVulkan.h"
	using RegionRenderer = RegionRendererVulkan;
#else
	#include "../world/WorldRendererGL.h"
	#include "../world/WorldRendererGLForward.h"
	#include "../world/WorldRendererGLDeferred.h"
	#include "../gui/GuiRendererGL.h"
	#include "../post/HDRRendererGL.h"
	#include "../post/PostProcessorGL.h"
	#include "../regions/RegionRendererGL.h"
	#include "../target/WindowRendererGL.h"
	using RegionRenderer = RegionRendererGL;
#endif
#include <y/EngineData.h>
#include <world/Camera.h>
#include <lib/os/msg.h>
#if __has_include(<lib/hui_minimal/hui.h>)
#include <lib/hui_minimal/hui.h>
#elif __has_include(<lib/hui/hui.h>)
#include <lib/hui/hui.h>
#endif
#include <helper/PerformanceMonitor.h>
#include <Config.h>



string render_graph_str(Renderer *r) {
	string s = PerformanceMonitor::get_name(r->channel);
	if (r->children.num == 1)
		s += " <<< " + render_graph_str(r->children[0]);
	if (r->children.num >= 2) {
		Array<string> ss;
		for (auto c: r->children)
			ss.add(render_graph_str(c));
		s += " <<< (" + implode(ss, ", ") + ")";
	}
	return s;
}

void print_render_chain() {
	msg_write("------------------------------------------");
	msg_write("CHAIN:  " + render_graph_str(engine.window_renderer));
	msg_write("------------------------------------------");
}


WindowRenderer *create_window_renderer(GLFWwindow* window) {
#ifdef HAS_LIB_GLFW
#ifdef USING_VULKAN
	return WindowRendererVulkan::create(window, device);
#else
	return new WindowRendererGL(window);
#endif
#else
	return nullptr;
#endif
}

Renderer *create_gui_renderer() {
#ifdef USING_VULKAN
	return new GuiRendererVulkan();
#else
	return new GuiRendererGL();
#endif
}

RegionRenderer *create_region_renderer() {
#ifdef USING_VULKAN
	return new RegionRendererVulkan();
#else
	return new RegionRendererGL();
#endif
}

HDRRenderer *create_hdr_renderer(Camera *cam) {
#ifdef USING_VULKAN
	return new HDRRendererVulkan(cam, engine.width, engine.height);
#else
	return new HDRRendererGL(cam, engine.width, engine.height);
#endif
}

PostProcessor *create_post_processor() {
#ifdef USING_VULKAN
	return new PostProcessorVulkan();
#else
	return new PostProcessorGL(engine.width, engine.height);
#endif
}

WorldRenderer *create_world_renderer(Camera *cam) {
#ifdef USING_VULKAN
	if (config.get_str("renderer.path", "forward") == "raytracing")
		return new WorldRendererVulkanRayTracing(device, cam, engine.width, engine.height);
	else
		return new WorldRendererVulkanForward(device, cam);
#else
	if (config.get_str("renderer.path", "forward") == "deferred")
		return new WorldRendererGLDeferred(cam, engine.width, engine.height);
	else
		return new WorldRendererGLForward(cam);
#endif
}

Renderer *create_render_path(Camera *cam) {
	if (config.get_str("renderer.path", "forward") == "direct") {
		engine.world_renderer = create_world_renderer(cam);
		return engine.world_renderer;
	} else {
	//	engine.post_processor = create_post_processor(parent);
	//	engine.hdr_renderer = create_hdr_renderer(engine.post_processor, cam);
		engine.hdr_renderer = create_hdr_renderer(cam);
		engine.world_renderer = create_world_renderer(cam);
		engine.hdr_renderer->add_child(engine.world_renderer);
		//post_processor->set_hdr(hdr_renderer);
		return engine.hdr_renderer;
	}
}

void create_full_renderer(GLFWwindow* window, Camera *cam) {
	try {
		engine.window_renderer = create_window_renderer(window);
		engine.region_renderer = create_region_renderer();
		auto p = create_render_path(cam);
		engine.gui_renderer = create_gui_renderer();
		engine.window_renderer->add_child(engine.region_renderer);
		engine.region_renderer->add_region(p, rect::ID, 0);
		engine.region_renderer->add_region(engine.gui_renderer, rect::ID, 999);
	} catch(Exception &e) {
#if __has_include(<lib/hui_minimal/hui.h>)
		hui::ShowError(e.message());
#endif
		throw e;
	}
	print_render_chain();
}
