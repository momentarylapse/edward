//
// Created by Michael Ankele on 2025-05-07.
//

#include "WorldParticlesEmitter.h"
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <lib/yrenderer/Context.h>
#include <fx/Particle.h>
#include <fx/Beam.h>
#include <fx/ParticleEmitter.h>
#include <lib/profiler/Profiler.h>
#include <world/Camera.h>
#include <lib/ygraphics/graphics-impl.h>
#include <y/EngineData.h>
#include <y/EntityManager.h>
#include <y/Entity.h>

using namespace yrenderer;
using namespace ygfx;

WorldParticlesEmitter::WorldParticlesEmitter(yrenderer::Context* ctx, Camera* _cam) :
		MeshEmitter(ctx, "fx"),
		fx_material(ctx)
{
	cam = _cam;
	fx_material.pass0.cull_mode = CullMode::NONE;
	fx_material.pass0.mode = TransparencyMode::FUNCTIONS;
	fx_material.pass0.source = Alpha::SOURCE_ALPHA;
	fx_material.pass0.destination = Alpha::SOURCE_INV_ALPHA;
	fx_material.pass0.shader_path = "fx.shader";

	fx_vertex_buffers.add(new VertexBuffer("3f,4f,2f"));
}

void WorldParticlesEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	if (shadow_pass)
		return;
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto shader = rvd.get_shader(&fx_material, 0, "fx", "");

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

	auto& legacy_particles = EntityManager::global->get_component_list_family<LegacyParticle>();
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

		rd.set_texture(BINDING_TEX0, texture);
		rd.draw_triangles(params, vb);
	}


	// new particles
	auto& particle_groups = EntityManager::global->get_component_list_family<ParticleGroup>();
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

		rd.set_texture(BINDING_TEX0, g->texture);
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
		rd.set_texture(BINDING_TEX0, g->texture);
		rd.draw_triangles(params, vb);
	}

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


