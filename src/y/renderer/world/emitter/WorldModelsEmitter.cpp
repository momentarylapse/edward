//
// Created by Michael Ankele on 2025-05-06.
//

#include "WorldModelsEmitter.h"
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <world/Model.h>
#include <world/components/Animator.h>
#include <ecs/EntityManager.h>
#include <ecs/Entity.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/base/sort.h>


static const string vertex_shader_module[2] = {"default", "animated"};

WorldOpaqueModelsEmitter::WorldOpaqueModelsEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "mod") {
}


void WorldOpaqueModelsEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto draw_model = [&params, &rvd, shadow_pass] (Entity* e, Model* m, const Array<yrenderer::Material*>& materials, Animator* ani) {
		for (int i=0; i<materials.num; i++) {
			auto material = materials[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and shadow_pass)
				continue;

			auto shader = rvd.get_shader(material, 0, vertex_shader_module[(int)(bool)ani], "");
			if (shadow_pass)
				material = rvd.material_shadow;

			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, e->get_matrix(), shader, material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
#ifdef USING_VULKAN
				rd.dset->set_uniform_buffer(yrenderer::BINDING_BONE_MATRICES, ani->buf);
#else
				nix::bind_uniform_buffer(yrenderer::BINDING_BONE_MATRICES, ani->buf);
#endif
			}

			rd.draw_triangles(params, vb);
		}
	};

	/*auto& list = EntityManager::global->get_component_list<Model>();
	for (auto m: list) {
		draw_model(m, m->materials, m->owner->get_component<Animator>());
	}*/

	auto& list2 = EntityManager::global->get_component_list<ModelRef>();
	for (auto mr: list2)
		if (mr->model) {
			mr->update_materials();
			draw_model(mr->owner, mr->model, mr->materials, mr->owner->get_component<Animator>());
		}

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

WorldTransparentModelsEmitter::WorldTransparentModelsEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "trans") {
}

void WorldTransparentModelsEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	if (shadow_pass)
		return;
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	struct DrawCallData {
		Entity* entity;
		Model* model;
		int material_index;
		yrenderer::Material* material;
		Animator* ani;
		float z;
	};
	Array<DrawCallData> draw_calls;

	auto maybe_add = [&] (Entity* e, Model* m, const Array<yrenderer::Material*>& materials, Animator* ani) {
		for (int i=0; i<materials.num; i++) {
			auto material = materials[i];
			if (!material->is_transparent())
				continue;

			// inside camera frustum?
			const float r = m->prop.radius;
			float zz = rvd.frustum[0].distance(e->pos); // also used for z-sorting
			if (zz < -r)
				continue;
			if (rvd.frustum[1].distance(e->pos) < -r)
				continue;
			if (rvd.frustum[2].distance(e->pos) < -r)
				continue;
			if (rvd.frustum[3].distance(e->pos) < -r)
				continue;
			if (rvd.frustum[4].distance(e->pos) < -r)
				continue;
			if (rvd.frustum[5].distance(e->pos) < -r)
				continue;

			draw_calls.add({e, m, i, material, ani, zz});
		}
	};

	/*auto& list = EntityManager::global->get_component_list<Model>();
	for (auto m: list)
		maybe_add(m, m->materials);*/
	auto& list2 = EntityManager::global->get_component_list<ModelRef>();
	for (auto mr: list2)
		if (auto m = mr->model) {
			mr->update_materials();
			maybe_add(mr->owner, m, mr->materials, mr->owner->get_component<Animator>());
		}

	// sort: far to near
	draw_calls = base::sorted(draw_calls, [] (const auto& a, const auto& b) { return a.z >= b.z; });


	// draw!
	for (const auto& dc: draw_calls) {
		auto m = dc.model;
		auto material = dc.material;
		int i = dc.material_index;

		auto vb = m->mesh[0]->sub[i].vertex_buffer;

		for (int k=0; k<material->num_passes; k++) {
			auto shader = rvd.get_shader(material, k, vertex_shader_module[(int)(bool)dc.ani], "");

			auto& rd = rvd.start(params, dc.entity->get_matrix(), shader, material, k, ygfx::PrimitiveTopology::TRIANGLES, vb);

			if (dc.ani) {
				dc.ani->buf->update_array(dc.ani->dmatrix);
#ifdef USING_VULKAN
				rd.dset->set_uniform_buffer(yrenderer::BINDING_BONE_MATRICES, dc.ani->buf);
#else
				nix::bind_uniform_buffer(yrenderer::BINDING_BONE_MATRICES, dc.ani->buf);
#endif
			}

			rd.draw_triangles(params, vb);
		}
	}
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


