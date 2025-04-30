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
#include "../../../world/Model.h"
#include "lib/base/sort.h"
#include "world/ModelManager.h"
#include "world/Terrain.h"
#include "world/World.h"
#include "world/components/Animator.h"
#include "world/components/MultiInstance.h"
#include "world/components/UserMesh.h"
#include "y/ComponentManager.h"
#include "y/Entity.h"

GeometryRenderer::GeometryRenderer(RenderPathType _type, SceneView &_scene_view) :
		Renderer("geo"),
		scene_view(_scene_view),
		fx_material(resource_manager)
{
	type = _type;
	flags = Flags::ALLOW_OPAQUE | Flags::ALLOW_TRANSPARENT;

	cur_rvd.type = type;
	cur_rvd.scene_view = &scene_view;

	ch_pre = PerformanceMonitor::create_channel("pre", channel);
	ch_bg = PerformanceMonitor::create_channel("bg", channel);
	ch_fx = PerformanceMonitor::create_channel("fx", channel);
	ch_terrains = PerformanceMonitor::create_channel("ter", channel);
	ch_models = PerformanceMonitor::create_channel("mod", channel);
	ch_user = PerformanceMonitor::create_channel("usr", channel);
	ch_prepare_lights = PerformanceMonitor::create_channel("lights", channel);

	fx_material.pass0.cull_mode = CullMode::NONE;
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

#ifdef USING_OPENGL
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);
	//cur_rvd.set_wire(wireframe);
#endif


	cur_rvd.begin_draw();
	if (override_view)
		cur_rvd.set_view_matrix(*override_view);
	else
		cur_rvd.set_view_matrix(scene_view.cam->view_matrix());
	if (override_projection)
		cur_rvd.set_projection_matrix(*override_projection);


	if ((int)(flags & Flags::ALLOW_CLEAR_COLOR))
		clear(params, cur_rvd);

	if ((int)(flags & Flags::ALLOW_SKYBOXES))
		draw_skyboxes(params, cur_rvd);

	if ((int)(flags & Flags::ALLOW_OPAQUE) or is_shadow_pass()) {
#ifdef USING_OPENGL
		if (!is_shadow_pass()) {
			nix::bind_uniform_buffer(BINDING_LIGHT, cur_rvd.ubo_light.get());
			nix::bind_texture(BINDING_SHADOW0, scene_view.shadow_maps[0]);
			nix::bind_texture(BINDING_SHADOW1, scene_view.shadow_maps[1]);
			nix::bind_texture(BINDING_CUBE, scene_view.cube_map.get());
		}
#endif
		draw_terrains(params, cur_rvd);
		draw_objects_opaque(params, cur_rvd);
		draw_objects_instanced(params, cur_rvd);
		draw_user_meshes(params, cur_rvd, false);
	}

	if ((int)(flags & Flags::ALLOW_TRANSPARENT)) {
#ifdef USING_OPENGL
		nix::bind_uniform_buffer(BINDING_LIGHT, cur_rvd.ubo_light.get());
		nix::bind_texture(BINDING_SHADOW0, scene_view.shadow_maps[0]);
		nix::bind_texture(BINDING_SHADOW1, scene_view.shadow_maps[1]);
		nix::bind_texture(BINDING_CUBE, scene_view.cube_map.get());
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

void GeometryRenderer::draw_skyboxes(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_bg);
	gpu_timestamp_begin(params, ch_bg);
	auto cam = scene_view.cam;
#ifdef USING_OPENGL
	nix::set_cull(nix::CullMode::NONE);
#endif

	float max_depth = cam->max_depth;
	float min_depth = cam->min_depth;
	cam->min_depth = 0.1f;
	cam->max_depth = 2000000;
	cam->update_matrices(params.desired_aspect_ratio);

	// overwrite rendering parameters
	auto mv = rvd.ubo.v;
	auto mp = rvd.ubo.p;
	rvd.set_view_matrix(mat4::rotation(cam->owner->ang).transpose());
	rvd.set_projection_matrix(mat4::scale(1,1,0.1f) * cam->m_projection); // :P

	// not working anymore... should have 2nd light data ubo
	int nlights = rvd.light_meta_data.num_lights;
	rvd.light_meta_data.num_lights = 0;

	for (auto *sb: world.skybox) {
		sb->_matrix = mat4::rotation(sb->owner->ang);

		for (int i=0; i<sb->material.num; i++) {
			auto vb = sb->mesh[0]->sub[i].vertex_buffer;
			auto shader = cur_rvd.get_shader(sb->material[i], 0, "default", "");
			auto& rd = rvd.start(params, sb->_matrix * mat4::scale(10,10,10), shader, *sb->material[i], 0, PrimitiveTopology::TRIANGLES, vb);

			rd.draw_triangles(params, vb);
		}
	}

	rvd.ubo.v = mv;
	rvd.ubo.p = mp;
	rvd.light_meta_data.num_lights = nlights;
	cam->min_depth = min_depth;
	cam->max_depth = max_depth;
	cam->update_matrices(params.desired_aspect_ratio);
#ifdef USING_OPENGL
	nix::set_cull(nix::CullMode::BACK);
#endif
	gpu_timestamp_end(params, ch_bg);
	PerformanceMonitor::end(ch_bg);
}


void GeometryRenderer::draw_terrains(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_terrains);
	gpu_timestamp_begin(params, ch_terrains);

	auto& terrains = ComponentManager::get_list_family<Terrain>();
	for (auto *t: terrains) {
		auto o = t->owner;

		auto material = t->material.get();
		if (is_shadow_pass() and !material->cast_shadow)
			return;
		auto shader = cur_rvd.get_shader(material, 0, t->vertex_shader_module, "");
		if (is_shadow_pass())
			material = cur_rvd.material_shadow;

		auto& rd = rvd.start(params, mat4::translation(o->pos), shader, *material, 0, PrimitiveTopology::TRIANGLES, t->vertex_buffer.get());

		if (!is_shadow_pass()) {
#ifdef USING_VULKAN
			params.command_buffer->push_constant(0, 4, &t->texture_scale[0].x);
			params.command_buffer->push_constant(4, 4, &t->texture_scale[1].x);
#else
			shader->set_floats("pattern0", &t->texture_scale[0].x, 3);
			shader->set_floats("pattern1", &t->texture_scale[1].x, 3);
#endif
		}
		rd.draw_triangles(params, t->vertex_buffer.get());
	}
	gpu_timestamp_end(params, ch_terrains);
	PerformanceMonitor::end(ch_terrains);
}

void GeometryRenderer::draw_objects_opaque(const RenderParams& params, RenderViewData &rvd) {
	//auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(params, ch_models);

	auto& list = ComponentManager::get_list_family<Model>();

	for (auto m: list) {
		auto ani = m->owner ? m->owner->get_component<Animator>() : nullptr;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and is_shadow_pass())
				continue;

			auto shader = cur_rvd.get_shader(material, 0, m->_template->vertex_shader_module, "");
			if (is_shadow_pass())
				material = cur_rvd.material_shadow;

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, m->_matrix, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
#ifdef USING_VULKAN
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#else
				nix::bind_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#endif
			}

			rd.draw_triangles(params, vb);
		}
	}
	gpu_timestamp_end(params, ch_models);
	PerformanceMonitor::end(ch_models);
}

void GeometryRenderer::draw_objects_instanced(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(params, ch_models);

	auto& list = ComponentManager::get_list_family<MultiInstance>();

	for (auto mi: list) {
		auto m = mi->model;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and is_shadow_pass())
				continue;

			auto shader = cur_rvd.get_shader(material, 0, "instanced", "");

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, mat4::ID, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);

#ifdef USING_VULKAN
			rd.dset->set_uniform_buffer(BINDING_INSTANCE_MATRICES, mi->ubo_matrices);
#else
			nix::bind_uniform_buffer(BINDING_INSTANCE_MATRICES, mi->ubo_matrices);
#endif

			rd.draw_instanced(params, vb, min(mi->matrices.num, MAX_INSTANCES));
		}
	}
	gpu_timestamp_end(params, ch_models);
	PerformanceMonitor::end(ch_models);
}

void GeometryRenderer::draw_objects_transparent(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(params, ch_models);
	auto cam = scene_view.cam;

	struct DrawCallData {
		Model* model;
		int material_index;
		float z;
	};
	Array<DrawCallData> draw_calls;

	auto& list = ComponentManager::get_list_family<Model>();

	for (auto m: list) {
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (!material->is_transparent())
				continue;

			draw_calls.add({m, i, (m->owner->pos - cam->owner->pos).length()});
		}
	}

	// sort: far to near
	draw_calls = base::sorted(draw_calls, [] (const auto& a, const auto& b) { return a.z >= b.z; });


	// draw!
	for (const auto& dc: draw_calls) {
		auto m = dc.model;
		auto material = dc.model->material[dc.material_index];
		int i = dc.material_index;
		auto ani = m->owner ? m->owner->get_component<Animator>() : nullptr;

		m->update_matrix();
		auto vb = m->mesh[0]->sub[i].vertex_buffer;

		for (int k=0; k<material->num_passes; k++) {
			auto shader = cur_rvd.get_shader(material, k, m->_template->vertex_shader_module, "");

			auto& rd = rvd.start(params, m->_matrix, shader, *material, k, PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
#ifdef USING_VULKAN
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#else
				nix::bind_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#endif
			}

			rd.draw_triangles(params, vb);
		}
	}
	gpu_timestamp_end(params, ch_models);
	PerformanceMonitor::end(ch_models);
}


void GeometryRenderer::draw_user_meshes(const RenderParams& params, RenderViewData &rvd, bool transparent) {
	PerformanceMonitor::begin(ch_user);
	gpu_timestamp_begin(params, ch_user);

	auto& meshes = ComponentManager::get_list_family<UserMesh>();

	for (auto m: meshes) {
		auto material = m->material.get();
		if (material->is_transparent() != transparent)
			continue;
		if (!material->cast_shadow and is_shadow_pass())
			continue;
		auto o = m->owner;

		if (is_shadow_pass())
			material = cur_rvd.material_shadow;

		auto vb = m->vertex_buffer.get();
		auto shader = cur_rvd.get_shader(material, 0, m->vertex_shader_module, m->geometry_shader_module);
		auto& rd = rvd.start(params, m->owner->get_matrix(), shader, *material, 0, m->topology, vb);

		rd.draw(params, m->vertex_buffer.get(), m->topology);
	}
	gpu_timestamp_end(params, ch_user);
	PerformanceMonitor::end(ch_user);
}

// keep this outside the drawing function, making sure it only gets called once per frame!
void GeometryRenderer::prepare_instanced_matrices() {
	PerformanceMonitor::begin(ch_pre);
	auto& list = ComponentManager::get_list_family<MultiInstance>();
	for (auto *mi: list) {
		if (!mi->ubo_matrices)
			mi->ubo_matrices = new UniformBuffer(MAX_INSTANCES * sizeof(mat4));
		//mi->ubo_matrices->update_array(mi->matrices);
		mi->ubo_matrices->update_part(&mi->matrices[0], 0, min(mi->matrices.num, MAX_INSTANCES) * sizeof(mat4));
	}
	PerformanceMonitor::end(ch_pre);
}
