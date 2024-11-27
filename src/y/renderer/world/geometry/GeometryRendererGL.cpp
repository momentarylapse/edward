/*
 * GeometryRendererGL.cpp
 *
 *  Created on: Dec 15, 2022
 *      Author: michi
 */

#include "GeometryRendererGL.h"

#ifdef USING_OPENGL
#include "../WorldRendererGL.h"
#include "../../base.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../world/Material.h"
#include "../../../Config.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../helper/ResourceManager.h"
#include "../../../plugins/PluginManager.h"
#include "../../../fx/Particle.h"
#include "../../../fx/Beam.h"
#include "../../../fx/ParticleManager.h"
#include "../../../fx/ParticleEmitter.h"
#include "../../../gui/gui.h"
#include "../../../gui/Picture.h"
#include "../../../world/Camera.h"
#include "../../../world/Material.h"
#include "../../../world/Model.h"
#include "../../../world/Terrain.h"
#include "../../../world/World.h"
#include "../../../world/Light.h"
#include "../../../world/ModelManager.h"
#include "../../../world/components/Animator.h"
#include "../../../world/components/UserMesh.h"
#include "../../../world/components/MultiInstance.h"
#include "../../../y/Entity.h"
#include "../../../y/ComponentManager.h"
#include "../../../meta.h"
#include <lib/base/sort.h>
#include <lib/nix/nix.h>
#include <lib/image/image.h>
#include <lib/math/vec3.h>
#include <lib/math/complex.h>
#include <lib/math/rect.h>
#include <lib/os/msg.h>


GeometryRendererGL::GeometryRendererGL(RenderPathType type, SceneView &scene_view) : GeometryRenderer(type, scene_view) {

	vb_fx = new nix::VertexBuffer("3f,4f,2f");
	vb_fx_points = new nix::VertexBuffer("3f,f,4f");

	//shader_fx = ResourceManager::load_shader("forward/3d-fx.shader");

	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)type];
	shader_fx = resource_manager->load_surface_shader("forward/3d-fx-uni.shader", rpt, "fx", "");
	shader_fx_points = resource_manager->load_surface_shader("forward/3d-fx-uni.shader", rpt, "points", "points");
}

void GeometryRendererGL::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);

	prepare_instanced_matrices();

	PerformanceMonitor::end(ch_prepare);
}

void GeometryRendererGL::set_material(const SceneView& scene_view, ShaderCache& cache, const Material& m, RenderPathType t, const string& vertex_module, const string& geometry_module) {
	cache._prepare_shader(t, m, vertex_module, geometry_module);
	set_material_x(scene_view, m, cache.get_shader(t));
}

void GeometryRendererGL::set_material_x(const SceneView& scene_view, const Material& m, Shader* s) {
	nix::set_shader(s);
	if (using_view_space)
		s->set_floats("eye_pos", &scene_view.cam->owner->pos.x, 3); // NAH....
	else
		s->set_floats("eye_pos", &vec3::ZERO.x, 3);
	s->set_int("num_lights", scene_view.lights.num);
	s->set_int("shadow_index", scene_view.shadow_index);
	for (auto &u: m.uniforms)
		s->set_floats(u.name, u.p, u.size/4);

	if (m.pass0.mode == TransparencyMode::FUNCTIONS)
		nix::set_alpha(m.pass0.source, m.pass0.destination);
	else if (m.pass0.mode == TransparencyMode::COLOR_KEY_HARD)
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	else if (m.pass0.mode == TransparencyMode::MIX)
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	else
		nix::disable_alpha();

	nix::bind_textures(weak(m.textures));
	nix::bind_texture(7, scene_view.cube_map.get());


	nix::set_material(m.albedo, m.roughness, m.metal, m.emission);
}


#if 0
void GeometryRendererGL::set_textures(const Array<Texture*> &tex) {
	auto tt = tex;
	if (tt.num == 0)
		tt.add(tex_white);
	if (tt.num == 1)
		tt.add(tex_white);
	if (tt.num == 2)
		tt.add(tex_white);
	/*tt.add(fb_shadow1->depth_buffer.get());
	tt.add(fb_shadow2->depth_buffer.get());
	tt.add(cube_map.get());*/
	nix::bind_textures(tt);
}
#endif

void create_color_quad(VertexBuffer *vb, const rect &d, const rect &s, const color &c) {
	Array<VertexFx> v = {{{d.x1,d.y1,0}, c, s.x1,s.y1},
	                    {{d.x1,d.y2,0}, c, s.x1,s.y2},
	                    {{d.x2,d.y2,0}, c, s.x2,s.y2},
	                    {{d.x1,d.y1,0}, c, s.x1,s.y1},
	                    {{d.x2,d.y2,0}, c, s.x2,s.y2},
	                    {{d.x2,d.y1,0}, c, s.x2,s.y1}};
	vb->update(v);
}


void GeometryRendererGL::draw_particles(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_fx);
	gpu_timestamp_begin(ch_fx);

	nix::set_shader(shader_fx.get());
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_z(false, true);
	nix::set_cull(nix::CullMode::NONE);

	auto cam = scene_view.cam;

	auto r = mat4::rotation(cam->owner->ang);

	auto add_beam_vertices = [&cam] (Array<VertexFx>& v, const vec3& pos, const vec3& length, float radius, const color& col, const rect& source) {
		// TODO geometry shader!
		auto pa = cam->project(pos);
		auto pb = cam->project(pos + length);
		auto pe = vec3::cross(pb - pa, vec3::EZ).normalized();
		auto uae = cam->unproject(pa + pe * 0.1f);
		auto ube = cam->unproject(pb + pe * 0.1f);
		auto _e1 = (pos - uae).normalized() * radius;
		auto _e2 = (pos + length - ube).normalized() * radius;
		//vec3 e1 = -vec3::cross(cam->ang * vec3::EZ, p.length).normalized() * p.radius/2;

		vec3 p00 = pos - _e1;
		vec3 p01 = pos - _e2 + length;
		vec3 p10 = pos + _e1;
		vec3 p11 = pos + _e2 + length;

		v.add({p00, col, source.x1, source.y1});
		v.add({p01, col, source.x2, source.y1});
		v.add({p11, col, source.x2, source.y2});
		v.add({p00, col, source.x1, source.y1});
		v.add({p11, col, source.x2, source.y2});
		v.add({p10, col, source.x1, source.y2});
	};

	// legacy particles
	base::map<Texture*, Array<LegacyParticle*>> legacy_groups;
	auto& legacy_particles = ComponentManager::get_list_family<LegacyParticle>();
	for (auto p: legacy_particles) {
		int i = legacy_groups.find(p->texture.get());
		if (i >= 0) {
			legacy_groups.by_index(i).add(p);
		} else {
			legacy_groups.add({p->texture.get(), {p}});
		}
	}

	for (const auto& [texture, particles]: legacy_groups) {
		nix::bind_texture(0, texture);

		Array<VertexFx> v;
		for (auto p: particles)
			if (p->enabled) {
				if (p->is_beam) {
					auto b = reinterpret_cast<LegacyBeam*>(p);
					add_beam_vertices(v, p->owner->pos, b->length, p->radius, p->col, p->source);
				} else {
					auto m = mat4::translation(p->owner->pos) * r * mat4::scale(p->radius, p->radius, p->radius);

					v.add({m * vec3(-1, 1, 0), p->col, p->source.x1, p->source.y1});
					v.add({m * vec3(1, 1, 0), p->col, p->source.x2, p->source.y1});
					v.add({m * vec3(1, -1, 0), p->col, p->source.x2, p->source.y2});
					v.add({m * vec3(-1, 1, 0), p->col, p->source.x1, p->source.y1});
					v.add({m * vec3(1, -1, 0), p->col, p->source.x2, p->source.y2});
					v.add({m * vec3(-1, -1, 0), p->col, p->source.x1, p->source.y2});
				}
			}
		vb_fx->update(v);
		nix::set_model_matrix(mat4::ID);
		nix::draw_triangles(vb_fx.get());
	}


	auto& particle_groups = ComponentManager::get_list_family<ParticleGroup>();
	for (auto g: particle_groups) {
		nix::bind_texture(0, g->texture);
		int count = 0;
		for (auto& p: g->particles)
			count += int(p.enabled);

		nix::set_shader(shader_fx_points.get());
		shader_fx_points->set_floats("source_uv", &g->source.x1, 4);
		Array<VertexPoint> v;
		v.__reserve(count);
		for (auto& p: g->particles)
			if (p.enabled)
				v.add({p.pos, p.radius*2, p.col});
		vb_fx_points->update(v);
		nix::set_model_matrix(mat4::ID);
		nix::draw_points(vb_fx_points.get());
	}

	// beams
	//Array<Vertex1> v = {{v_0, v_0, 0,0}, {v_0, v_0, 0,1}, {v_0, v_0, 1,1}, {v_0, v_0, 0,0}, {v_0, v_0, 1,1}, {v_0, v_0, 1,0}};
	nix::set_shader(shader_fx.get());
	nix::set_model_matrix(mat4::ID);
	for (auto g: particle_groups) {
		nix::bind_texture(0, g->texture);
		auto source = g->source;

		Array<VertexFx> v;

		for (auto& b: g->beams)
			if (b.enabled)
				add_beam_vertices(v, b.pos, b.length, b.radius, b.col, source);

		vb_fx->update(v);
		nix::draw_triangles(vb_fx.get());
	}


	nix::set_z(true, true);
	nix::disable_alpha();
	gpu_timestamp_end(ch_fx);
	PerformanceMonitor::end(ch_fx);
}

void GeometryRendererGL::draw_skyboxes(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_bg);
	gpu_timestamp_begin(ch_bg);
	nix::set_z(false, false);
	nix::set_cull(nix::CullMode::NONE);
	nix::set_view_matrix(mat4::rotation(scene_view.cam->owner->ang).transpose());
	for (auto *sb: world.skybox) {
		sb->_matrix = mat4::rotation(sb->owner->ang);
		nix::set_model_matrix(sb->_matrix * mat4::scale(10,10,10));
		for (int i=0; i<sb->material.num; i++) {
			set_material(scene_view, sb->shader_cache[i], *sb->material[i], type, "default", "");
			nix::draw_triangles(sb->mesh[0]->sub[i].vertex_buffer);
		}
	}
	nix::set_cull(nix::CullMode::BACK);
	nix::disable_alpha();
	gpu_timestamp_end(ch_bg);
	PerformanceMonitor::end(ch_bg);
}

void GeometryRendererGL::draw_terrains(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_terrains);
	gpu_timestamp_begin(ch_terrains);
	auto& terrains = ComponentManager::get_list_family<Terrain>();
	for (auto *t: terrains) {
		if (!t->material->cast_shadow and is_shadow_pass())
			continue;
		auto o = t->owner;
		nix::set_model_matrix(mat4::translation(o->pos));
		if (is_shadow_pass()) {
			set_material(scene_view, t->shader_cache_shadow, *material_shadow, type, t->vertex_shader_module, "");
		} else {
			set_material(scene_view, t->shader_cache, *t->material.get(), type, t->vertex_shader_module, "");
			auto s = t->shader_cache.get_shader(type);
			s->set_floats("pattern0", &t->texture_scale[0].x, 3);
			s->set_floats("pattern1", &t->texture_scale[1].x, 3);
		}
		nix::draw_triangles(t->vertex_buffer.get());
	}
	gpu_timestamp_end(ch_terrains);
	PerformanceMonitor::end(ch_terrains);
}

void GeometryRendererGL::draw_objects_instanced(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(ch_models);
	auto& list = ComponentManager::get_list_family<MultiInstance>();
	for (auto *mi: list) {
		auto m = mi->model;
		for (int i=0; i<m->material.num; i++) {
			if (!m->material[i]->cast_shadow and is_shadow_pass())
				continue;
			nix::set_model_matrix(mi->matrices[0]);//m->_matrix);
			if (is_shadow_pass()) {
				set_material(scene_view, m->shader_cache_shadow[i], *material_shadow, type, "instanced", "");
			} else {
				set_material(scene_view, m->shader_cache[i], *m->material[i], type, "instanced", "");
			}
			nix::bind_uniform_buffer(5, mi->ubo_matrices);
			//msg_write(s.matrices.num);
			nix::draw_instanced_triangles(m->mesh[0]->sub[i].vertex_buffer, mi->matrices.num);
		}
	}
	gpu_timestamp_end(ch_models);
	PerformanceMonitor::end(ch_models);
}

void GeometryRendererGL::draw_objects_opaque(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(ch_models);
	auto& list = ComponentManager::get_list_family<Model>();
	for (auto *m: list) {
		m->update_matrix();
		nix::set_model_matrix(m->_matrix);

		if (auto ani = m->owner->get_component<Animator>()) {
			ani->buf->update_array(ani->dmatrix);
			nix::bind_uniform_buffer(7, ani->buf);
		}

		for (int i=0; i<m->material.num; i++) {
			if (m->material[i]->is_transparent())
				continue;
			if (!m->material[i]->cast_shadow and is_shadow_pass())
				continue;

			if (is_shadow_pass()) {
				set_material(scene_view, m->shader_cache_shadow[i], *material_shadow, type, m->_template->vertex_shader_module, "");
			} else {
				set_material(scene_view, m->shader_cache[i], *weak(m->material)[i], type, m->_template->vertex_shader_module, "");
			}
			nix::draw_triangles(m->mesh[0]->sub[i].vertex_buffer);
		}
	}
	gpu_timestamp_end(ch_models);
	PerformanceMonitor::end(ch_models);
}

struct DrawCallDataMultiPass {
	mat4 matrix;
	VertexBuffer *vb;
	Material *material;
	Array<Shader*> shaders;
	float z;
};

void GeometryRendererGL::draw_objects_transparent(const RenderParams& params, RenderViewData &rvd) {
	if (is_shadow_pass())
		return;
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(ch_models);
	nix::set_z(false, true);

	Array<DrawCallDataMultiPass> draw_calls;

	auto& list = ComponentManager::get_list_family<Model>();
	for (auto *m: list) {
		m->update_matrix();

		/*if (auto ani = m->owner->get_component<Animator>()) {
			ani->buf->update_array(ani->dmatrix);
			nix::bind_uniform_buffer(7, ani->buf);
		}*/

		for (int i=0; i<m->material.num; i++) {
			if (!m->material[i]->is_transparent())
				continue;

			auto material = weak(m->material)[i];
			Array<Shader*> shaders;
			for (int k=0; k<material->num_passes; k++) {
				auto &p = material->pass(k);
				if (!multi_pass_shader_cache[k].contains(material))
					multi_pass_shader_cache[k].set(material, {});
				auto &shader_cache = multi_pass_shader_cache[k][material];

				shader_cache._prepare_shader_multi_pass(type, *material, m->_template->vertex_shader_module, "", k);
				shaders.add(shader_cache.get_shader(type));
			}

			draw_calls.add({m->_matrix, m->mesh[0]->sub[i].vertex_buffer, material, shaders, (m->owner->pos - scene_view.cam->owner->pos).length()});
		}
	}

	// sort: far to near
	draw_calls = base::sorted(draw_calls, [] (const auto& a, const auto& b) { return a.z > b.z; });

	// draw!
	for (const auto& dc: draw_calls) {
		nix::set_model_matrix(dc.matrix);
		for (int k=0; k<dc.material->num_passes; k++) {
			auto& p = dc.material->pass(k);

			set_material_x(scene_view, *dc.material, dc.shaders[k]);

			if (p.mode == TransparencyMode::FUNCTIONS)
				nix::set_alpha(p.source, p.destination);
			else if (p.mode == TransparencyMode::COLOR_KEY_HARD)
				nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
			else if (p.mode == TransparencyMode::MIX)
				nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
			else
				nix::disable_alpha();

			if (p.cull_mode == 0)
				nix::set_cull(nix::CullMode::NONE);
			else if (p.cull_mode == 2)
				nix::set_cull(nix::CullMode::FRONT);
			else
				nix::set_cull(nix::CullMode::BACK);

			nix::draw_triangles(dc.vb);
		}
	}

	nix::disable_alpha();
	nix::set_z(true, true);
	nix::set_cull(nix::CullMode::BACK);
	gpu_timestamp_end(ch_models);
	PerformanceMonitor::end(ch_models);
}

void GeometryRendererGL::draw_user_meshes(const RenderParams& params, RenderViewData &rvd, bool transparent) {
	PerformanceMonitor::begin(ch_user);
	gpu_timestamp_begin(ch_user);
	auto& meshes = ComponentManager::get_list_family<UserMesh>();
	for (auto *m: meshes) {
		if (m->material->is_transparent() != transparent)
			continue;
		if (!m->material->cast_shadow and is_shadow_pass())
			continue;
		auto o = m->owner;
		nix::set_model_matrix(o->get_matrix());


		if (is_shadow_pass()) {
			set_material(scene_view, m->shader_cache_shadow, *material_shadow, type, m->vertex_shader_module, "");
		} else {
			set_material(scene_view, m->shader_cache, *m->material.get(), type, m->vertex_shader_module, "");
		}

		
		if (m->topology == PrimitiveTopology::TRIANGLES)
			nix::draw_triangles(m->vertex_buffer.get());
		else if (m->topology == PrimitiveTopology::POINTS)
			nix::draw_points(m->vertex_buffer.get());
		else if (m->topology == PrimitiveTopology::LINES)
			nix::draw_lines(m->vertex_buffer.get(), false);
		else if (m->topology == PrimitiveTopology::LINESTRIP)
			nix::draw_lines(m->vertex_buffer.get(), true);
	}
	gpu_timestamp_end(ch_user);
	PerformanceMonitor::end(ch_user);
}


void GeometryRendererGL::prepare_instanced_matrices() {
	PerformanceMonitor::begin(ch_pre);
	auto& list = ComponentManager::get_list_family<MultiInstance>();
	for (auto *mi: list) {
		if (!mi->ubo_matrices)
			mi->ubo_matrices = new nix::UniformBuffer();
		mi->ubo_matrices->update_array(mi->matrices);
	}
	PerformanceMonitor::end(ch_pre);
}



void GeometryRendererGL::draw_opaque(const RenderParams& params, RenderViewData &rvd) {
	if (!is_shadow_pass()) {
		nix::set_z(true, true);
		nix::set_view_matrix(scene_view.cam->view_matrix());
		nix::bind_uniform_buffer(1, rvd.ubo_light.get());
		nix::bind_texture(3, scene_view.fb_shadow1->depth_buffer.get());
		nix::bind_texture(4, scene_view.fb_shadow2->depth_buffer.get());
		nix::bind_texture(5, scene_view.cube_map.get());
	}

	// opaque
	draw_terrains(params, rvd);
	draw_objects_instanced(params, rvd);
	draw_objects_opaque(params, rvd);
	draw_user_meshes(params, rvd, false);
}

void GeometryRendererGL::draw_transparent(const RenderParams& params, RenderViewData &rvd) {
	nix::set_view_matrix(scene_view.cam->view_matrix());
	//nix::set_z(true, true);

	nix::bind_uniform_buffer(1, rvd.ubo_light.get());
	nix::bind_texture(3, scene_view.fb_shadow1->depth_buffer.get());
	nix::bind_texture(4, scene_view.fb_shadow2->depth_buffer.get());
	nix::bind_texture(5, scene_view.cube_map.get());

	draw_objects_transparent(params, rvd);
	draw_user_meshes(params, rvd, true);
	draw_particles(params, rvd);
}


#endif
