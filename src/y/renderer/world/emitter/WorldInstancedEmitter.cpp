//
// Created by Michael Ankele on 2025-05-08.
//

#include "WorldInstancedEmitter.h"
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <world/components/MultiInstance.h>
#include <world/Model.h>
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <ecs/EntityManager.h>
#include <lib/ygraphics/graphics-impl.h>

using namespace yrenderer;

class MultiInstance;

WorldInstancedEmitter::WorldInstancedEmitter(Context* ctx) : MeshEmitter(ctx, "inst") {
}

void WorldInstancedEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto& list = EntityManager::global->get_component_list<MultiInstance>();

	for (auto mi: list) {
		auto m = mi->model;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and shadow_pass)
				continue;

			auto shader = rvd.get_shader(material, 0, "instanced", "");

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, mat4::ID, shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);

#ifdef USING_VULKAN
			rd.dset->set_uniform_buffer(BINDING_INSTANCE_MATRICES, mi->ubo_matrices);
#else
			nix::bind_uniform_buffer(BINDING_INSTANCE_MATRICES, mi->ubo_matrices);
#endif

			rd.draw_instanced(params, vb, min(mi->matrices.num, MAX_INSTANCES));
		}
	}
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

