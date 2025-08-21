//
// Created by Michael Ankele on 2025-05-08.
//

#include "WorldUserMeshesEmitter.h"
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/Bindable.h>
#include <world/components/UserMesh.h>
#include <y/ComponentManager.h>
#include <y/Entity.h>

WorldOpaqueUserMeshesEmitter::WorldOpaqueUserMeshesEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "user") {
}

void WorldOpaqueUserMeshesEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto& meshes = ComponentManager::get_list_family<UserMesh>();

	for (auto m: meshes) {
		auto material = m->material.get();
		//if (material->is_transparent() != transparent)
		//	continue;
		if ((m->topology != ygfx::PrimitiveTopology::TRIANGLES) and shadow_pass) // FIXME workaround for bug?
			continue;
		if (!material->cast_shadow and shadow_pass)
			continue;

		if (shadow_pass)
			material = rvd.material_shadow;

		auto vb = m->vertex_buffer.get();
		auto shader = rvd.get_shader(material, 0, m->vertex_shader_module, m->geometry_shader_module);
		auto& rd = rvd.start(params, m->owner->get_matrix(), shader, *material, 0, m->topology, vb);
		yrenderer::apply_shader_data(params, shader, m->material->shader_data);

		rd.draw(params, m->vertex_buffer.get(), m->topology);
	}
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

WorldTransparentUserMeshesEmitter::WorldTransparentUserMeshesEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "user*") {
}

void WorldTransparentUserMeshesEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
}



