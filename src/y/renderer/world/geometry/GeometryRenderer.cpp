/*
 * GeometryRenderer.cpp
 *
 *  Created on: Dec 15, 2022
 *      Author: michi
 */

#include "GeometryRenderer.h"
#include "SceneView.h"
#include "../../base.h"
#include "../../../graphics-impl.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../helper/ResourceManager.h"
#include "../../../world/Camera.h"

GeometryRenderer::GeometryRenderer(RenderPathType _type, SceneView &_scene_view) :
		Renderer("geo"),
		scene_view(_scene_view),
		fx_material(resource_manager)
{
	type = _type;
	flags = Flags::ALLOW_OPAQUE | Flags::ALLOW_TRANSPARENT;

	cur_rvd.scene_view = &scene_view;

	ch_pre = PerformanceMonitor::create_channel("pre", channel);
	ch_bg = PerformanceMonitor::create_channel("bg", channel);
	ch_fx = PerformanceMonitor::create_channel("fx", channel);
	ch_terrains = PerformanceMonitor::create_channel("ter", channel);
	ch_models = PerformanceMonitor::create_channel("mod", channel);
	ch_user = PerformanceMonitor::create_channel("usr", channel);
	ch_prepare_lights = PerformanceMonitor::create_channel("lights", channel);

	fx_material.pass0.cull_mode = 0;
	fx_material.pass0.mode = TransparencyMode::FUNCTIONS;
	fx_material.pass0.source = Alpha::SOURCE_ALPHA;
	fx_material.pass0.destination = Alpha::SOURCE_INV_ALPHA;
	fx_material.pass0.shader_path = "fx.shader";

	fx_vertex_buffers.add(new VertexBuffer("3f,4f,2f"));

#ifdef USING_OPENGL
	vb_fx = new nix::VertexBuffer("3f,4f,2f");
	vb_fx_points = new nix::VertexBuffer("3f,f,4f");

	//shader_fx = ResourceManager::load_shader("forward/3d-fx.shader");

	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)type];
	shader_fx = resource_manager->load_surface_shader("forward/3d-fx-uni.shader", rpt, "fx", "");
	shader_fx_points = resource_manager->load_surface_shader("forward/3d-fx-uni.shader", rpt, "points", "points");
#endif
}

void GeometryRenderer::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);

	prepare_instanced_matrices();

	PerformanceMonitor::end(ch_prepare);
}


Shader* GeometryRenderer::get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module) {
	if (!multi_pass_shader_cache[pass_no].contains(material))
		multi_pass_shader_cache[pass_no].set(material, {});
	auto& cache = multi_pass_shader_cache[pass_no][material];
	if (is_shadow_pass())
		cache._prepare_shader_multi_pass(type, *material_shadow, vertex_shader_module, geometry_shader_module, pass_no);
	else
		cache._prepare_shader_multi_pass(type, *material, vertex_shader_module, geometry_shader_module, pass_no);
	return cache.get_shader(type);
}

void GeometryRenderer::set(Flags _flags) {
	flags = _flags;
}

bool GeometryRenderer::is_shadow_pass() const {
	return (int)(flags & Flags::SHADOW_PASS);
}

void GeometryRenderer::draw(const RenderParams& params) {
	bool flip_y = params.target_is_window;

#ifdef USING_OPENGL
	auto m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
#else
	auto m = mat4::ID;
#endif
	//	if (config.antialiasing_method == AntialiasingMethod::TAA)
	//		 m *= jitter(fb->width, fb->height, 0);

	//scene_view.cam->update_matrices(params.desired_aspect_ratio);
	//cur_rvd.set_projection_matrix(scene_view.cam->m_projection * m);
	cur_rvd.ubo.p = cur_rvd.ubo.p * m;
	//cur_rvd.set_view_matrix(scene_view.cam->m_view);
	cur_rvd.ubo.num_lights = scene_view.lights.num;
	cur_rvd.ubo.shadow_index = scene_view.shadow_index;

#ifdef USING_OPENGL
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);
	//cur_rvd.set_wire(wireframe);
#endif


	cur_rvd.begin_draw();


	if ((int)(flags & Flags::ALLOW_CLEAR_COLOR))
		clear(params, cur_rvd);

	if ((int)(flags & Flags::ALLOW_SKYBOXES))
		draw_skyboxes(params, cur_rvd);

	if ((int)(flags & Flags::ALLOW_OPAQUE) or is_shadow_pass()) {
#ifdef USING_OPENGL
		if (!is_shadow_pass()) {
			nix::set_z(true, true);
			nix::set_view_matrix(scene_view.cam->view_matrix());
			nix::bind_uniform_buffer(1, cur_rvd.ubo_light.get());
			nix::bind_texture(3, scene_view.shadow_maps[0]);
			nix::bind_texture(4, scene_view.shadow_maps[1]);
			nix::bind_texture(5, scene_view.cube_map.get());
		}
#endif
		draw_terrains(params, cur_rvd);
		draw_objects_opaque(params, cur_rvd);
		draw_objects_instanced(params, cur_rvd);
		draw_user_meshes(params, cur_rvd, false);
	}

	if ((int)(flags & Flags::ALLOW_TRANSPARENT)) {
#ifdef USING_OPENGL
		nix::set_view_matrix(scene_view.cam->view_matrix());
		//nix::set_z(true, true);

		nix::bind_uniform_buffer(1, cur_rvd.ubo_light.get());
		nix::bind_texture(3, scene_view.shadow_maps[0]);
		nix::bind_texture(4, scene_view.shadow_maps[1]);
		nix::bind_texture(5, scene_view.cube_map.get());
#endif
		draw_objects_transparent(params, cur_rvd);
		draw_particles(params, cur_rvd);
		draw_user_meshes(params, cur_rvd, true);
	}

#ifdef USING_VULKAN
#else
	cur_rvd.set_cull(CullMode::BACK);
	nix::set_front(nix::Orientation::CW);
	//nix::set_scissor(rect::EMPTY);
	cur_rvd.set_wire(false);
#endif
}
