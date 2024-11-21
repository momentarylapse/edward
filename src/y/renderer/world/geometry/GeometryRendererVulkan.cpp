/*
 * GeometryRendererVulkan.cpp
 *
 *  Created on: Dec 16, 2022
 *      Author: michi
 */

#include "GeometryRendererVulkan.h"

#ifdef USING_VULKAN
#include "../WorldRendererVulkan.h"
#include "../../helper/PipelineManager.h"
#include "../../base.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../world/Material.h"
#include "../../../Config.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../helper/ResourceManager.h"
#include "../../../fx/Particle.h"
#include "../../../fx/Beam.h"
#include "../../../fx/ParticleEmitter.h"
#include "../../../fx/ParticleManager.h"
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
#include <lib/image/image.h>
#include <lib/math/vec3.h>
#include <lib/math/complex.h>
#include <lib/math/rect.h>
#include <lib/os/msg.h>



const int BINDING_TEX0 = 0;
const int BINDING_SHADOW0 = 5;
const int BINDING_SHADOW1 = 6;
const int BINDING_CUBE = 7;
const int BINDING_PARAMS = 8;
const int BINDING_LIGHT = 9;
const int BINDING_INSTANCE_MATRICES = 10;
const int BINDING_BONE_MATRICES = 11;

const int MAX_LIGHTS = 1024;
const int MAX_INSTANCES = 1<<11;

RenderViewDataVK::RenderViewDataVK() {
	ubo_light = new UniformBuffer(MAX_LIGHTS * sizeof(UBOLight));
}

RenderDataVK& RenderViewDataVK::start(
		const RenderParams& params, RenderPathType type, const mat4& matrix,
		ShaderCache& shader_cache, const Material& material, int pass_no,
		const string& vertex_shader_module, const string& geometry_shader_module,
		PrimitiveTopology top, VertexBuffer *vb) {
	shader_cache._prepare_shader_multi_pass(type, material, vertex_shader_module, geometry_shader_module, pass_no);
	if (index >= rda.num) {
		rda.add({new UniformBuffer(sizeof(UBO)),
		            pool->create_set(shader_cache.get_shader(type))});
		rda[index].dset->set_uniform_buffer(BINDING_PARAMS, rda[index].ubo);
		rda[index].dset->set_uniform_buffer(BINDING_LIGHT, ubo_light.get());
	}

	ubo.m = matrix;
	ubo.albedo = material.albedo;
	ubo.emission = material.emission;
	ubo.metal = material.metal;
	ubo.roughness = material.roughness;
	rda[index].ubo->update_part(&ubo, 0, sizeof(UBO));

	auto s = shader_cache.get_shader(type);
	auto p = GeometryRendererVulkan::get_pipeline(s, params.render_pass, material.pass(pass_no), top, vb);

	params.command_buffer->bind_pipeline(p);

	if (scene_view)
		rda[index].set_textures(*scene_view, weak(material.textures));

	return rda[index ++];
}

void RenderDataVK::set_textures(const SceneView& scene_view, const Array<Texture*>& tex) {
	foreachi (auto t, tex, i)
						if (t)
							dset->set_texture(BINDING_TEX0 + i, t);
	if (scene_view.fb_shadow1)
		dset->set_texture(BINDING_SHADOW0, scene_view.fb_shadow1->attachments[1].get());
	if (scene_view.fb_shadow1)
		dset->set_texture(BINDING_SHADOW1, scene_view.fb_shadow2->attachments[1].get());
	if (scene_view.cube_map)
		dset->set_texture(BINDING_CUBE, scene_view.cube_map.get());
}

void RenderDataVK::apply(const RenderParams& params) {
	dset->update();
	params.command_buffer->bind_descriptor_set(0, dset);
}


GeometryRendererVulkan::GeometryRendererVulkan(RenderPathType type, SceneView &scene_view) : GeometryRenderer(type, scene_view) {
}

void GeometryRendererVulkan::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(channel);

	prepare_instanced_matrices();

	PerformanceMonitor::end(channel);
}


vulkan::CullMode vk_cull(int culling) {
	if (culling == 0)
		return vulkan::CullMode::NONE;
	if (culling == 2)
		return vulkan::CullMode::FRONT;
	if (culling == 1)
		return vulkan::CullMode::BACK;
	return vulkan::CullMode::NONE;
}

GraphicsPipeline* GeometryRendererVulkan::get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb) {
	if (pass.mode == TransparencyMode::FUNCTIONS) {
		return PipelineManager::get_alpha(s, rp, top, vb, pass.source, pass.destination, false, vk_cull(pass.cull_mode));
	} else if (pass.mode == TransparencyMode::COLOR_KEY_HARD) {
		return PipelineManager::get_alpha(s, rp, top, vb, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, true, vk_cull(pass.cull_mode));
	} else {
		return PipelineManager::get(s, rp, top, vb);
	}
}



void GeometryRendererVulkan::draw_particles(const RenderParams& params, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_fx);
	gpu_timestamp_begin(cb, ch_fx);
	auto cam = scene_view.cam;

	auto& rd = rvd.start(params, type, mat4::ID, fx_shader_cache, fx_material, 0, "fx", "", PrimitiveTopology::TRIANGLES, fx_vertex_buffers[0]);

	// particles
	auto r = mat4::rotation(cam->owner->ang);
	int index = 0;

	auto get_vb = [this, &index]() {
		if (index < fx_vertex_buffers.num)
			return fx_vertex_buffers[index ++];
		index ++;
		fx_vertex_buffers.add(new VertexBuffer("3f,4f,2f"));
		return fx_vertex_buffers.back();
	};

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

		Array<VertexFx> v;
		v.__reserve(particles.num * 6);
		for (auto p: particles)
			if (p->enabled) {
				if (p->is_beam) {
					auto b = reinterpret_cast<LegacyBeam*>(p);
					auto pa = cam->project(p->owner->pos);
					auto pb = cam->project(p->owner->pos + b->length);
					auto pe = vec3::cross(pb - pa, vec3::EZ).normalized();
					auto uae = cam->unproject(pa + pe * 0.1f);
					auto ube = cam->unproject(pb + pe * 0.1f);
					auto _e1 = (p->owner->pos - uae).normalized() * p->radius;
					auto _e2 = (p->owner->pos + b->length - ube).normalized() * p->radius;
					//vec3 e1 = -vec3::cross(cam->ang * vec3::EZ, p.length).normalized() * p.radius/2;

					vec3 p00 = p->owner->pos - _e1;
					vec3 p01 = p->owner->pos - _e2 + b->length;
					vec3 p10 = p->owner->pos + _e1;
					vec3 p11 = p->owner->pos + _e2 + b->length;

					v.add({p00, p->col, p->source.x1, p->source.y1});
					v.add({p01, p->col, p->source.x2, p->source.y1});
					v.add({p11, p->col, p->source.x2, p->source.y2});
					v.add({p00, p->col, p->source.x1, p->source.y1});
					v.add({p11, p->col, p->source.x2, p->source.y2});
					v.add({p10, p->col, p->source.x1, p->source.y2});
				} else {
					auto m = mat4::translation(p->owner->pos) * r * mat4::scale(p->radius, p->radius, p->radius);

					v.add({m * vec3(-1, 1,0), p->col, p->source.x1, p->source.y1});
					v.add({m * vec3( 1, 1,0), p->col, p->source.x2, p->source.y1});
					v.add({m * vec3( 1,-1,0), p->col, p->source.x2, p->source.y2});
					v.add({m * vec3(-1, 1,0), p->col, p->source.x1, p->source.y1});
					v.add({m * vec3( 1,-1,0), p->col, p->source.x2, p->source.y2});
					v.add({m * vec3(-1,-1,0), p->col, p->source.x1, p->source.y2});
				}
			}
		auto vb = get_vb();
		vb->update(v);

		rd.dset->set_texture(BINDING_TEX0, texture);
		rd.apply(params);
		cb->draw(vb);
	}


	// new particles
	auto& particle_groups = ComponentManager::get_list_family<ParticleGroup>();
	for (auto g: particle_groups) {
		auto source = g->source;
		Array<VertexFx> v;
		for (auto& p: g->particles)
			if (p.enabled) {
				auto m = mat4::translation(p.pos) * r * mat4::scale(p.radius, p.radius, p.radius);
				v.add({m * vec3(-1, 1,0), p.col, source.x1, source.y1});
				v.add({m * vec3( 1, 1,0), p.col, source.x2, source.y1});
				v.add({m * vec3( 1,-1,0), p.col, source.x2, source.y2});
				v.add({m * vec3(-1, 1,0), p.col, source.x1, source.y1});
				v.add({m * vec3( 1,-1,0), p.col, source.x2, source.y2});
				v.add({m * vec3(-1,-1,0), p.col, source.x1, source.y2});
			}
		auto vb = get_vb();
		vb->update(v);

		rd.dset->set_texture(BINDING_TEX0, g->texture);
		rd.apply(params);
		cb->draw(vb);
	}

	// beams
	for (auto g: particle_groups) {
		if (g->beams.num == 0)
			continue;

		auto source = g->source;
		Array<VertexFx> v;
		for (auto& p: g->beams) {
			if (!p.enabled)
				continue;
			// TODO geometry shader!
			auto pa = cam->project(p.pos);
			auto pb = cam->project(p.pos + p.length);
			auto pe = vec3::cross(pb - pa, vec3::EZ).normalized();
			auto uae = cam->unproject(pa + pe * 0.1f);
			auto ube = cam->unproject(pb + pe * 0.1f);
			auto _e1 = (p.pos - uae).normalized() * p.radius;
			auto _e2 = (p.pos + p.length - ube).normalized() * p.radius;
			//vec3 e1 = -vec3::cross(cam->ang * vec3::EZ, p.length).normalized() * p.radius/2;

			vec3 p00 = p.pos - _e1;
			vec3 p01 = p.pos - _e2 + p.length;
			vec3 p10 = p.pos + _e1;
			vec3 p11 = p.pos + _e2 + p.length;

			v.add({p00, p.col, source.x1, source.y1});
			v.add({p01, p.col, source.x2, source.y1});
			v.add({p11, p.col, source.x2, source.y2});
			v.add({p00, p.col, source.x1, source.y1});
			v.add({p11, p.col, source.x2, source.y2});
			v.add({p10, p.col, source.x1, source.y2});
		}
		auto vb = get_vb();
		vb->update(v);
		rd.dset->set_texture(BINDING_TEX0, g->texture);
		cb->draw(vb);
	}

	gpu_timestamp_end(cb, ch_fx);
	PerformanceMonitor::end(ch_fx);
}

void GeometryRendererVulkan::draw_skyboxes(const RenderParams& params, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_bg);
	gpu_timestamp_begin(cb, ch_bg);
	auto cam = scene_view.cam;

	float max_depth = cam->max_depth;
	cam->max_depth = 2000000;
	cam->update_matrices(params.desired_aspect_ratio);

	// overwrite rendering parameters
	auto mv = rvd.ubo.v;
	auto mp = rvd.ubo.p;
	rvd.ubo.v = mat4::rotation(cam->owner->ang).transpose();
	rvd.ubo.p = cam->m_projection;
	int nlights = rvd.ubo.num_lights;
	rvd.ubo.num_lights = 0;

	for (auto *sb: world.skybox) {
		sb->_matrix = mat4::rotation(sb->owner->ang);

		for (int i=0; i<sb->material.num; i++) {

			auto vb = sb->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, type, sb->_matrix * mat4::scale(10,10,10), sb->shader_cache[i], *sb->material[i], 0, "default", "", PrimitiveTopology::TRIANGLES, vb);

			rd.apply(params);
			cb->draw(vb);
		}
	}

	// restore parameters
	rvd.ubo.v = mv;
	rvd.ubo.p = mp;
	rvd.ubo.num_lights = nlights;
	cam->max_depth = max_depth;
	cam->update_matrices(params.desired_aspect_ratio);
	gpu_timestamp_end(cb, ch_bg);
	PerformanceMonitor::end(ch_bg);
}

void GeometryRendererVulkan::draw_terrains(const RenderParams& params, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_terrains);
	gpu_timestamp_begin(cb, ch_terrains);

	auto& terrains = ComponentManager::get_list_family<Terrain>();
	for (auto *t: terrains) {
		auto o = t->owner;

		auto material = t->material.get();
		auto shader_cache = &t->shader_cache;
		if (is_shadow_pass()) {
			material = material_shadow;
			shader_cache = &t->shader_cache_shadow;
		}

		auto& rd = rvd.start(params, type, mat4::translation(o->pos), *shader_cache, *material, 0, t->vertex_shader_module, "", PrimitiveTopology::TRIANGLES, t->vertex_buffer.get());

		if (!is_shadow_pass()) {
			cb->push_constant(0, 4, &t->texture_scale[0].x);
			cb->push_constant(4, 4, &t->texture_scale[1].x);
		}
		rd.apply(params);
		cb->draw(t->vertex_buffer.get());
	}
	gpu_timestamp_end(cb, ch_terrains);
	PerformanceMonitor::end(ch_terrains);
}

void GeometryRendererVulkan::draw_objects_instanced(const RenderParams& params, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(cb, ch_models);

	auto& list = ComponentManager::get_list_family<MultiInstance>();

	for (auto mi: list) {
		auto m = mi->model;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and is_shadow_pass())
				continue;

			auto shader_cache = &m->shader_cache[i];
			if (is_shadow_pass()) {
				material = material_shadow;
				shader_cache = &m->shader_cache_shadow[i];
			}

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, type, mat4::ID, *shader_cache, *material, 0, "instanced", "", PrimitiveTopology::TRIANGLES, vb);

			rd.dset->set_uniform_buffer(BINDING_INSTANCE_MATRICES, mi->ubo_matrices);

			rd.apply(params);
			cb->draw_instanced(vb, min(mi->matrices.num, MAX_INSTANCES));
		}
	}
	gpu_timestamp_end(cb, ch_models);
	PerformanceMonitor::end(ch_models);
}



void GeometryRendererVulkan::draw_objects_opaque(const RenderParams& params, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(cb, ch_models);

	auto& list = ComponentManager::get_list_family<Model>();

	for (auto m: list) {
		auto ani = m->owner ? m->owner->get_component<Animator>() : nullptr;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and is_shadow_pass())
				continue;

			auto shader_cache = &m->shader_cache[i];
			if (is_shadow_pass()) {
				material = material_shadow;
				shader_cache = &m->shader_cache_shadow[i];
			}

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, type, m->_matrix, *shader_cache, *material, 0, m->_template->vertex_shader_module, "", PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
			}

			rd.apply(params);
			cb->draw(vb);
		}
	}
	gpu_timestamp_end(cb, ch_models);
	PerformanceMonitor::end(ch_models);
}


void GeometryRendererVulkan::draw_objects_transparent(const RenderParams& params, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_models);
	gpu_timestamp_begin(cb, ch_models);
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
			if (!multi_pass_shader_cache[k].contains(material))
				multi_pass_shader_cache[k].set(material, {});
			auto &shader_cache = multi_pass_shader_cache[k][material];

			auto& rd = rvd.start(params, type, m->_matrix, shader_cache, *material, k, m->_template->vertex_shader_module, "", PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
			}

			rd.apply(params);
			cb->draw(vb);
		}
	}
	gpu_timestamp_end(cb, ch_models);
	PerformanceMonitor::end(ch_models);
}

void GeometryRendererVulkan::draw_user_meshes(const RenderParams& params, bool transparent, RenderViewDataVK &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_user);
	gpu_timestamp_begin(cb, ch_user);

	auto& meshes = ComponentManager::get_list_family<UserMesh>();

	for (auto m: meshes) {
		if (!m->material->cast_shadow and is_shadow_pass())
			continue;
		if (m->material->is_transparent() != transparent)
			continue;

		auto material = m->material.get();
		auto shader_cache = &m->shader_cache;
		if (is_shadow_pass()) {
			material = material_shadow;
			shader_cache = &m->shader_cache_shadow;
		}

		auto vb = m->vertex_buffer.get();
		auto& rd = rvd.start(params, type, m->owner->get_matrix(), *shader_cache, *material, 0, m->vertex_shader_module, m->geometry_shader_module, m->topology, vb);

		rd.apply(params);
		cb->draw(m->vertex_buffer.get());
	}
	gpu_timestamp_end(cb, ch_user);
	PerformanceMonitor::end(ch_user);
}

// keep this outside the drawing function, making sure it only gets called once per frame!
void GeometryRendererVulkan::prepare_instanced_matrices() {
	PerformanceMonitor::begin(ch_pre);
	auto& list = ComponentManager::get_list_family<MultiInstance>();
	for (auto *mi: list) {
		if (!mi->ubo_matrices)
			mi->ubo_matrices = new UniformBuffer(MAX_INSTANCES * sizeof(mat4));
		mi->ubo_matrices->update_part(&mi->matrices[0], 0, min(mi->matrices.num, MAX_INSTANCES) * sizeof(mat4));
	}
	PerformanceMonitor::end(ch_pre);
}



#endif
