//
// Created by Michael Ankele on 2025-05-06.
//

#include "WorldModelsEmitter.h"
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <world/Model.h>
#include <world/ModelManager.h>
#include <world/components/Animator.h>
#include <y/ComponentManager.h>
#include <y/Entity.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/base/sort.h>

WorldOpaqueModelsEmitter::WorldOpaqueModelsEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "mod") {
}


void WorldOpaqueModelsEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto& list = ComponentManager::get_list_family<Model>();

	for (auto m: list) {
		auto ani = m->owner ? m->owner->get_component<Animator>() : nullptr;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and shadow_pass)
				continue;

			auto shader = rvd.get_shader(material, 0, m->_template->vertex_shader_module, "");
			if (shadow_pass)
				material = rvd.material_shadow;

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, m->_matrix, shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);

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

			draw_calls.add({m, i, (m->owner->pos - rvd.camera_params.pos).length()});
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
			auto shader = rvd.get_shader(material, k, m->_template->vertex_shader_module, "");

			auto& rd = rvd.start(params, m->_matrix, shader, *material, k, ygfx::PrimitiveTopology::TRIANGLES, vb);

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
	}
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


