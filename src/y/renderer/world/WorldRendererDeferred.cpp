/*
 * WorldRenderer.cpp
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#include "WorldRendererDeferred.h"

//#ifdef USING_OPENGL
#include "geometry/GeometryRenderer.h"
#include "../target/TextureRenderer.h"
#include "pass/ShadowRenderer.h"
#include "../post/ThroughShaderRenderer.h"
#include "../base.h"
#include "../path/RenderPath.h"
#include <lib/nix/nix.h>
#include <lib/os/msg.h>
#include <lib/math/random.h>
#include <lib/math/vec4.h>
#include <lib/math/vec2.h>

#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../plugins/ControllerManager.h"
#include "../../plugins/PluginManager.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../world/World.h"
#include "../../y/Entity.h"
#include "../../Config.h"
#include "../../meta.h"
#include "../../graphics-impl.h"


WorldRendererDeferred::WorldRendererDeferred(SceneView& scene_view, int width, int height) : WorldRenderer("world/def", scene_view) {

	auto tex1 = new Texture(width, height, "rgba:f16"); // diffuse
	auto tex2 = new Texture(width, height, "rgba:f16"); // emission
	auto tex3 = new Texture(width, height, "rgba:f16"); // pos
	auto tex4 = new Texture(width, height, "rgba:f16"); // normal,reflectivity
	auto depth = new DepthBuffer(width, height, "ds:u24i8");
	gbuffer_textures = {tex1, tex2, tex3, tex4, depth};
	for (auto a: weak(gbuffer_textures))
		a->set_options("wrap=clamp,magfilter=nearest,minfilter=nearest");


	gbuffer_renderer = new TextureRenderer("gbuf", gbuffer_textures);
	gbuffer_renderer->clear_z = true;
	gbuffer_renderer->clear_colors = {color(-1, 0,1,0)};


	resource_manager->load_shader_module("forward/module-surface.shader");
	resource_manager->load_shader_module("deferred/module-surface.shader");

	auto shader_gbuffer_out = resource_manager->load_shader("deferred/out.shader");
//	if (!shader_gbuffer_out->link_uniform_block("SSAO", 13))
//		msg_error("SSAO");

	out_renderer = new ThroughShaderRenderer("out", shader_gbuffer_out);
	out_renderer->bind_textures(0, {tex1, tex2, tex3, tex4, depth});


	Array<vec4> ssao_samples;
	Random r;
	for (int i=0; i<64; i++) {
		auto v = r.dir() * pow(r.uniform01(), 1);
		ssao_samples.add(vec4(v.x, v.y, abs(v.z), 0));
	}
	ssao_sample_buffer = new UniformBuffer(ssao_samples.num * sizeof(vec4));
	ssao_sample_buffer->update_array(ssao_samples);

	ch_gbuf_out = PerformanceMonitor::create_channel("gbuf-out", channel);
	ch_trans = PerformanceMonitor::create_channel("trans", channel);

	geo_renderer_background = new GeometryRenderer(RenderPathType::Forward, scene_view);
	geo_renderer_background->set(GeometryRenderer::Flags::ALLOW_SKYBOXES | GeometryRenderer::Flags::ALLOW_CLEAR_COLOR);
	add_child(geo_renderer_background.get());

	geo_renderer = new GeometryRenderer(RenderPathType::Deferred, scene_view);
	geo_renderer->set(GeometryRenderer::Flags::ALLOW_OPAQUE);
	gbuffer_renderer->add_child(geo_renderer.get());

	geo_renderer_trans = new GeometryRenderer(RenderPathType::Forward, scene_view);
	geo_renderer_trans->set(GeometryRenderer::Flags::ALLOW_TRANSPARENT);
	add_child(geo_renderer_trans.get());
}

void WorldRendererDeferred::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);


	auto sub_params = params.with_target(gbuffer_renderer->frame_buffer.get());

	gbuffer_renderer->set_area(dynamicly_scaled_area(gbuffer_renderer->frame_buffer.get()));
	gbuffer_renderer->prepare(params);


	scene_view.cam->update_matrices(params.desired_aspect_ratio);

	//geo_renderer_background->cur_rvd.set_view_matrix()
	geo_renderer_background->cur_rvd.set_projection_matrix(scene_view.cam->m_projection);
	geo_renderer_background->cur_rvd.set_view_matrix(scene_view.cam->m_view);
	geo_renderer_background->cur_rvd.prepare_scene(&scene_view);
	geo_renderer_background->prepare(params); // keep drawing into direct target
	geo_renderer->prepare(sub_params);
	geo_renderer_trans->prepare(params); // keep drawing into direct target


	gbuffer_renderer->render(params);

	PerformanceMonitor::end(ch_prepare);
}

void WorldRendererDeferred::draw(const RenderParams& params) {
	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);

	geo_renderer_background->draw(params);

	render_out_from_gbuffer(gbuffer_renderer->frame_buffer.get(), params);

	// transparency
#ifdef USING_OPENGL
	auto& rvd = geo_renderer_trans->cur_rvd;

	PerformanceMonitor::begin(ch_trans);
	bool flip_y = params.target_is_window;
	mat4 m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
	auto cam = scene_view.cam;
	cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(m * cam->m_projection); // TODO
	nix::bind_uniform_buffer(BINDING_LIGHT, rvd.ubo_light.get());
	nix::set_view_matrix(cam->view_matrix()); // TODO
	nix::set_z(true, true);
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);

	geo_renderer_trans->draw(params);
	nix::set_cull(nix::CullMode::BACK);
	nix::set_front(nix::Orientation::CW);

	nix::set_z(false, false);
	//nix::set_projection_matrix(mat4::ID);
	//nix::set_view_matrix(mat4::ID);
	PerformanceMonitor::end(ch_trans);
#endif

	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}

void WorldRendererDeferred::render_out_from_gbuffer(FrameBuffer *source, const RenderParams& params) {
	PerformanceMonitor::begin(ch_gbuf_out);

	auto& data = out_renderer->bindings.shader_data;

#ifdef USING_OPENGL
	if constexpr (GeometryRenderer::using_view_space)
		data.dict_set("eye_pos", vec3_to_any(vec3::ZERO));
	else
		data.dict_set("eye_pos", vec3_to_any(scene_view.cam->owner->pos)); // NAH
#endif
	data.dict_set("ambient_occlusion_radius:8", config.ambient_occlusion_radius);
	out_renderer->bind_uniform_buffer(13, ssao_sample_buffer);

	auto& rvd = geo_renderer->cur_rvd;
	out_renderer->bind_uniform_buffer(BINDING_LIGHT, rvd.ubo_light.get());
	for (int i=0; i<gbuffer_textures.num; i++)
		out_renderer->bind_texture(i, gbuffer_textures[i].get());
	out_renderer->bind_texture(BINDING_SHADOW0, scene_view.shadow_maps[0]);
	out_renderer->bind_texture(BINDING_SHADOW1, scene_view.shadow_maps[1]);

	float resolution_scale_x = 1.0f;
	data.dict_set("resolution_scale:0", vec2_to_any(vec2(resolution_scale_x, resolution_scale_x)));

	out_renderer->set_source(dynamicly_scaled_source());
	out_renderer->draw(params);

	// ...
	//geo_renderer->draw_transparent();

	PerformanceMonitor::end(ch_gbuf_out);
}

//void WorldRendererDeferred::render_into_texture(nix::FrameBuffer *fb, Camera *cam) {}

