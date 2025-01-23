/*
 * GeometryRendererVulkan.cpp
 *
 *  Created on: Dec 16, 2022
 *      Author: michi
 */

#include "GeometryRenderer.h"

#ifdef USING_VULKAN
#include "RenderViewData.h"
#include "SceneView.h"
#include "../../helper/PipelineManager.h"
#include "../../base.h"
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






vulkan::CullMode vk_cull(int culling) {
	if (culling == 0)
		return vulkan::CullMode::NONE;
	if (culling == 2)
		return vulkan::CullMode::FRONT;
	if (culling == 1)
		return vulkan::CullMode::BACK;
	return vulkan::CullMode::NONE;
}

GraphicsPipeline* GeometryRenderer::get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb) {
	if (pass.mode == TransparencyMode::FUNCTIONS) {
		return PipelineManager::get_alpha(s, rp, top, vb, pass.source, pass.destination, false, vk_cull(pass.cull_mode));
	} else if (pass.mode == TransparencyMode::COLOR_KEY_HARD) {
		return PipelineManager::get_alpha(s, rp, top, vb, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, true, vk_cull(pass.cull_mode));
	} else {
		return PipelineManager::get(s, rp, top, vb, vk_cull(pass.cull_mode));
	}
}



void GeometryRenderer::draw_particles(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_fx);
	gpu_timestamp_begin(params, ch_fx);
	auto cam = scene_view.cam;

	auto shader = cur_rvd.get_shader(&fx_material, 0, "fx", "");

	auto& rd = rvd.start(params, mat4::ID, shader, fx_material, 0, PrimitiveTopology::TRIANGLES, fx_vertex_buffers[0]);

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

	gpu_timestamp_end(params, ch_fx);
	PerformanceMonitor::end(ch_fx);
}

void GeometryRenderer::clear(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
	cb->clear(params.frame_buffer->area(), {world.background}, 1.0f);
}

void GeometryRenderer::draw_skyboxes(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_bg);
	gpu_timestamp_begin(params, ch_bg);
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
			auto shader = cur_rvd.get_shader(sb->material[i], 0, "default", "");
			auto& rd = rvd.start(params, sb->_matrix * mat4::scale(10,10,10), shader, *sb->material[i], 0, PrimitiveTopology::TRIANGLES, vb);

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
	gpu_timestamp_end(params, ch_bg);
	PerformanceMonitor::end(ch_bg);
}

void GeometryRenderer::draw_terrains(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
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
			cb->push_constant(0, 4, &t->texture_scale[0].x);
			cb->push_constant(4, 4, &t->texture_scale[1].x);
		}
		rd.apply(params);
		cb->draw(t->vertex_buffer.get());
	}
	gpu_timestamp_end(params, ch_terrains);
	PerformanceMonitor::end(ch_terrains);
}

void GeometryRenderer::draw_objects_instanced(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
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

			rd.dset->set_uniform_buffer(BINDING_INSTANCE_MATRICES, mi->ubo_matrices);

			rd.apply(params);
			cb->draw_instanced(vb, min(mi->matrices.num, MAX_INSTANCES));
		}
	}
	gpu_timestamp_end(params, ch_models);
	PerformanceMonitor::end(ch_models);
}



void GeometryRenderer::draw_objects_opaque(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
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
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
			}

			rd.apply(params);
			cb->draw(vb);
		}
	}
	gpu_timestamp_end(params, ch_models);
	PerformanceMonitor::end(ch_models);
}


void GeometryRenderer::draw_objects_transparent(const RenderParams& params, RenderViewData &rvd) {
	auto cb = params.command_buffer;
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
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
			}

			rd.apply(params);
			cb->draw(vb);
		}
	}
	gpu_timestamp_end(params, ch_models);
	PerformanceMonitor::end(ch_models);
}

void GeometryRenderer::draw_user_meshes(const RenderParams& params, RenderViewData &rvd, bool transparent) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_user);
	gpu_timestamp_begin(params, ch_user);

	auto& meshes = ComponentManager::get_list_family<UserMesh>();

	for (auto m: meshes) {
		if (!m->material->cast_shadow and is_shadow_pass())
			continue;
		if (m->material->is_transparent() != transparent)
			continue;

		auto material = m->material.get();
		if (is_shadow_pass())
			material = cur_rvd.material_shadow;

		auto vb = m->vertex_buffer.get();
		auto shader = cur_rvd.get_shader(material, 0, m->vertex_shader_module, m->geometry_shader_module);
		auto& rd = rvd.start(params, m->owner->get_matrix(), shader, *material, 0, m->topology, vb);

		rd.apply(params);
		cb->draw(m->vertex_buffer.get());
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
		mi->ubo_matrices->update_part(&mi->matrices[0], 0, min(mi->matrices.num, MAX_INSTANCES) * sizeof(mat4));
	}
	PerformanceMonitor::end(ch_pre);
}




#endif
