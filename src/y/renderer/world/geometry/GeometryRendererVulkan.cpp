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
	if (pass.mode == TransparencyMode::FUNCTIONS)
		return PipelineManager::get_alpha(s, rp, top, vb, pass.source, pass.destination, vk_cull(pass.cull_mode), pass.z_test, pass.z_buffer);
	if (pass.mode == TransparencyMode::COLOR_KEY_HARD)
		return PipelineManager::get_alpha(s, rp, top, vb, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, vk_cull(pass.cull_mode), pass.z_test, pass.z_buffer);
	return PipelineManager::get(s, rp, top, vb, vk_cull(pass.cull_mode), pass.z_test, pass.z_buffer);
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
		rd.draw_triangles(params, vb);
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
		rd.draw_triangles(params, vb);
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




#endif
