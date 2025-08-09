/*
 * RenderPath.cpp
 *
 *  Created on: Jan 19, 2020
 *      Author: michi
 */

#include "RenderPath.h"
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/pass/ShadowRenderer.h>
#include <lib/yrenderer/scene/pass/CubeMapRenderer.h>
#include <lib/ygraphics/graphics-impl.h>

namespace yrenderer {

/*struct GeoPush {
	alignas(16) mat4 model;
	alignas(16) color emission;
	alignas(16) vec3 eye_pos;
	alignas(16) float xxx[4];
};


mat4 mtr(const vec3 &t, const quaternion &a) {
	auto mt = mat4::translation(t);
	auto mr = mat4::rotation(a);
	return mt * mr;
}*/

string RenderPath::light_sources_module = "default";
string RenderPath::lighting_method = "pbr";
string RenderPath::shadow_method = "pcf-hardening";

RenderPath::RenderPath(Context* ctx, const string &name) : Renderer(ctx, name) {
	background_color = color(0.3f, 0.3f, 0.3f, 1);

	if (ctx) {
		shader_manager->default_shader = "default.shader";
		ctx->load_shader_module("module-basic-interface.shader");
		ctx->load_shader_module("module-basic-data.shader");
		ctx->load_shader_module(format("module-light-sources-%s.shader", light_sources_module));
		ctx->load_shader_module(format("module-shadows-%s.shader", shadow_method));
		ctx->load_shader_module(format("module-lighting-%s.shader", lighting_method));
		ctx->load_shader_module("module-vertex-default.shader");
		ctx->load_shader_module("module-vertex-animated.shader");
		ctx->load_shader_module("module-vertex-instanced.shader");
		ctx->load_shader_module("module-vertex-lines.shader");
		ctx->load_shader_module("module-vertex-points.shader");
		ctx->load_shader_module("module-vertex-fx.shader");
		ctx->load_shader_module("module-geometry-lines.shader");
		ctx->load_shader_module("module-geometry-points.shader");
	}
}

RenderPath::~RenderPath() = default;

void RenderPath::set_view(const CameraParams& _view) {
	view = _view;
	scene_view.main_camera_params = view;
}

void RenderPath::set_lights(const Array<Light*>& lights) {
	scene_view.choose_lights(lights);
	scene_view.choose_shadows();
}


void RenderPath::reset() {
}

void RenderPath::create_shadow_renderer(int resolution) {
	shadow_renderer = new ShadowRenderer(ctx, &scene_view, resolution);
	scene_view.shadow_maps.add(shadow_renderer->cascades[0].depth_buffer);
	scene_view.shadow_maps.add(shadow_renderer->cascades[1].depth_buffer);
	//add_sub_task(shadow_renderer.get());
}

void RenderPath::create_cube_renderer() {
	cube_map_renderer = new CubeMapRenderer(ctx, scene_view);
}

void RenderPath::render_into_cubemap(const RenderParams& params, CubeMapSource& source) {
	cube_map_renderer->set_source(&source);
	cube_map_renderer->render(params);
}

}
