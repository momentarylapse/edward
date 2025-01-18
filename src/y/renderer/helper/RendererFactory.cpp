/*
 * RendererFactory.cpp
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#include "RendererFactory.h"
#include "../base.h"
#include "../path/RenderPath.h"
#include "../world/WorldRenderer.h"
#include "../post/ThroughShaderRenderer.h"
#include "../regions/RegionRenderer.h"
#ifdef USING_VULKAN
	#include "../gui/GuiRendererVulkan.h"
	#include "../post/PostProcessorVulkan.h"
	#include "../target/WindowRendererVulkan.h"
#else
	#include "../gui/GuiRendererGL.h"
	#include "../post/PostProcessorGL.h"
	#include "../target/WindowRendererGL.h"
#endif
#include <y/EngineData.h>
#include <lib/os/msg.h>
#if __has_include(<lib/hui_minimal/hui.h>)
#include <lib/hui_minimal/hui.h>
#elif __has_include(<lib/hui/hui.h>)
#include <lib/hui/hui.h>
#endif
#include <helper/PerformanceMonitor.h>
#include <Config.h>
#include <helper/ResourceManager.h>

// for debugging
#include <graphics-impl.h>
#include <lib/image/image.h>
#include <renderer/target/TextureRenderer.h>


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
	return new RegionRenderer();
}

PostProcessor *create_post_processor() {
#ifdef USING_VULKAN
	return new PostProcessorVulkan();
#else
	return new PostProcessorGL(engine.width, engine.height);
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

void create_and_attach_render_path(Camera *cam) {
	auto rp = create_render_path(cam);
	engine.render_paths.add(rp);
	engine.region_renderer->add_region(rp, rect::ID, 0);
}


void create_base_renderer(GLFWwindow* window) {
	try {
		engine.window_renderer = create_window_renderer(window);
		engine.region_renderer = create_region_renderer();
		engine.gui_renderer = create_gui_renderer();
		engine.window_renderer->add_child(engine.region_renderer);
		engine.region_renderer->add_region(engine.gui_renderer, rect::ID, 999);

		if (false) {
			int N = 256;
			Image im;
			im.create(N, N, Black);
			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
					im.set_pixel(i, j, ((i/16+j/16)%2 == 0) ? Black : White);
			shared tex = new Texture();
			tex->write(im);
			auto shader = engine.resource_manager->load_shader("forward/blur.shader");
			auto tsr = new ThroughShaderRenderer("blur", shader);
			tsr->bind_texture(0, tex.get());
			Any axis_x, axis_y;
			axis_x.list_set(0, 1.0f);
			axis_x.list_set(1, 0.0f);
			axis_y.list_set(0, 0.0f);
			axis_y.list_set(1, 1.0f);
			Any data;
			data.dict_set("radius:8", 5.0f);
			data.dict_set("threshold:12", 0.0f);
			data.dict_set("axis:0", axis_x);
			tsr->bindings.shader_data = data;
			// tsr:  tex -> shader -> ...

			shared tex2 = new Texture(N, N, "rgba:i8");
#ifdef USING_VULKAN
			shared<Texture> depth2 = new DepthBuffer(N, N, "d:f32", true);
#else
			shared<Texture> depth2 = new DepthBuffer(N, N, "d24s8");
#endif
			auto tr = new TextureRenderer("tex", {tex2, depth2});
			//tr->use_params_area = false;
			tr->add_child(tsr);
			// tr:  ... -> tex2

			auto tsr2 = new ThroughShaderRenderer("text", shader);
			tsr2->bind_texture(0, tex2.get());
			data.dict_set("radius:8", 5.0f);
			data.dict_set("threshold:12", 0.0f);
			data.dict_set("axis:0", axis_y);
			tsr2->bindings.shader_data = data;
			tsr2->add_child(tr);
			// tsr2:  tex2 -> shader -> ...

			engine.window_renderer->add_child(tsr2);
		}

	} catch(Exception &e) {
#if __has_include(<lib/hui_minimal/hui.h>)
		hui::ShowError(e.message());
#endif
		throw e;
	}
	print_render_chain();
}
