/*
 * ShadowRenderer.cpp
 *
 *  Created on: Dec 11, 2022
 *      Author: michi
 */

#include "ShadowRenderer.h"

#include "../geometry/GeometryRenderer.h"
#include "../../target/TextureRenderer.h"
#include <lib/os/msg.h>
#include <renderer/path/RenderPath.h>
#include "../WorldRenderer.h"
#include "../../base.h"
#include "../../../graphics-impl.h"
#include "../../../helper/PerformanceMonitor.h"
#include <world/Material.h>
#include <world/Camera.h>
#include "../../../Config.h"


ShadowRenderer::Cascade::Cascade() = default;
ShadowRenderer::Cascade::~Cascade() = default;

ShadowRenderer::ShadowRenderer(Camera* cam) :
		RenderTask("shdw")
{
	//int shadow_box_size = config.get_float("shadow.boxsize", 2000);
	int shadow_resolution = config.get_int("shadow.resolution", 1024);

	material = new Material(resource_manager);
	material->pass0.shader_path = "shadow.shader";

	scene_view.cam = cam;
	scene_view.shadow_indices.clear();

	for (int i=0; i<NUM_CASCADES; i++) {
		auto& c = cascades[i];
		c.geo_renderer = new GeometryRenderer(RenderPathType::Forward, scene_view);
		c.geo_renderer->flags = GeometryRenderer::Flags::SHADOW_PASS;
		c.geo_renderer->cur_rvd.material_shadow = material.get();

		shared tex = new Texture(shadow_resolution, shadow_resolution, "rgba:i8");
		c.depth_buffer = new DepthBuffer(shadow_resolution, shadow_resolution, "d:f32");
		c.texture_renderer = new TextureRenderer(format("cas%d", i), {tex, c.depth_buffer}, {"autoclear"});
		c.scale = (i == 0) ? 4.0f : 1.0f;
		c.texture_renderer->add_child(c.geo_renderer.get());
	}
}

void ShadowRenderer::set_projection(const mat4& proj) {
	for (int i=0; i<NUM_CASCADES; i++) {
		auto& c = cascades[i];

#ifdef USING_OPENGL
		auto m = mat4::scale(c.scale, c.scale, 1);
#else
		auto m = mat4::scale(c.scale, -c.scale, 1);
#endif
		c.geo_renderer->override_view = mat4::ID;
		c.geo_renderer->override_projection = m * proj;
	}
}

void ShadowRenderer::render(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	gpu_timestamp_begin(params, ch_prepare);

	render_cascade(params, cascades[0]);
	render_cascade(params, cascades[1]);

	gpu_timestamp_end(params, ch_prepare);
	PerformanceMonitor::end(ch_prepare);
}

void ShadowRenderer::render_cascade(const RenderParams& _params, Cascade& c) {
	auto params = _params.with_target(c.texture_renderer->frame_buffer.get());
	params.desired_aspect_ratio = 1.0f;


	// all opaque meshes
	c.geo_renderer->prepare(params);
	c.texture_renderer->render(params);
}
