/*
 * RendererFactory.cpp
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#include "RendererFactory.h"
#include <lib/yrenderer/Context.h>
#include "../FullCameraRenderer.h"
#include <lib/yrenderer/scene/path/RenderPath.h>
#include <lib/yrenderer/helper/CubeMapSource.h>
#include <lib/yrenderer/post/ThroughShaderRenderer.h>
#include <lib/yrenderer/regions/RegionRenderer.h>
#include <lib/yrenderer/target/WindowRenderer.h>
#include "../gui/GuiRenderer.h"
#include <lib/yrenderer/post/PostProcessor.h>
#include <EngineData.h>
#include <lib/os/msg.h>
#include <lib/profiler/Profiler.h>
#include <Config.h>
#include <lib/yrenderer/target/VrRenderer.h>


using namespace yrenderer;
using namespace ygfx;

string render_graph_str(Renderer *r) {
	string s = profiler::get_name(r->channel);
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

void print_render_graph(Renderer* root) {
	msg_write("------------------------------------------");
	msg_write("CHAIN:  " + render_graph_str(root));
	msg_write("------------------------------------------");
}


WindowRenderer *create_window_renderer(yrenderer::Context* ctx, GLFWwindow* window) {
#ifdef HAS_LIB_GLFW
	return new WindowRenderer(ctx, window, true);
#else
	return nullptr;
#endif
}

Renderer *create_gui_renderer(yrenderer::Context* ctx) {
	return new GuiRenderer(ctx);
}

RegionRenderer *create_region_renderer(yrenderer::Context* ctx) {
	return new RegionRenderer(ctx);
}

PostProcessor *create_post_processor(yrenderer::Context* ctx) {
#ifdef USING_VULKAN
	return new PostProcessor(ctx);
#else
	return new PostProcessor(ctx, engine.width, engine.height);
#endif
}

/*class TextureWriter : public Renderer {
public:
	shared<Texture> texture;
	TextureWriter(shared<Texture> t) : Renderer("www") {
		texture = t;
	}
	void prepare(const RenderParams& params) override {
		Renderer::prepare(params);

		Image i;
		texture->read(i);
		i.save("o.bmp");
	}
};*/

void create_and_attach_camera_renderer(yrenderer::Context* ctx, Camera *cam) {
	auto cr = create_camera_renderer(ctx, cam);
	engine.camera_renderers.add(cr);
	engine.region_renderer->add_region(cr, rect::ID, 0);
}


void create_base_renderer(yrenderer::Context* ctx, GLFWwindow* window) {
	cubemap_default_resolution = config.cubemap_resolution;
	cubemap_default_rate = config.cubemap_update_rate;

	try {
		engine.window_renderer = create_window_renderer(ctx, window);
		engine.region_renderer = create_region_renderer(ctx);
		engine.gui_renderer = create_gui_renderer(ctx);
		engine.window_renderer->add_child(engine.region_renderer);
		engine.region_renderer->add_region(engine.gui_renderer, rect::ID, 999);
	} catch(Exception &e) {
		msg_error(e.message());
		throw;
	}
	print_render_graph(engine.window_renderer);
}


void create_base_renderer_vr(yrenderer::Context* ctx) {
#ifdef HAS_VR_RENDERER
	cubemap_default_resolution = config.cubemap_resolution;
	cubemap_default_rate = config.cubemap_update_rate;

	try {
		engine.vr_renderer = new VrRenderer(ctx);
		engine.region_renderer = create_region_renderer(ctx);
		engine.gui_renderer = create_gui_renderer(ctx);
		engine.vr_renderer->add_child(engine.region_renderer);
		engine.region_renderer->add_region(engine.gui_renderer, rect::ID, 999);
	} catch(Exception &e) {
		msg_error(e.message());
		throw;
	}
	print_render_graph(engine.vr_renderer);
#endif
}
