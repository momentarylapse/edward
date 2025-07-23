//
// Created by Michael Ankele on 2025-05-08.
//

#include "WorldUserMeshesEmitter.h"
#include "../../scene/RenderViewData.h"
#include <lib/profiler/Profiler.h>
#include <renderer/base.h>
#include <world/components/UserMesh.h>
#include <y/ComponentManager.h>
#include <y/Entity.h>

WorldUserMeshesEmitter::WorldUserMeshesEmitter() : MeshEmitter("user") {
}

void WorldUserMeshesEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	gpu_timestamp_begin(params, channel);

	auto& meshes = ComponentManager::get_list_family<UserMesh>();

	for (auto m: meshes) {
		auto material = m->material.get();
		//if (material->is_transparent() != transparent)
		//	continue;
		if (!material->cast_shadow and shadow_pass)
			continue;
		auto o = m->owner;

		if (shadow_pass)
			material = rvd.material_shadow;

		auto vb = m->vertex_buffer.get();
		auto shader = rvd.get_shader(material, 0, m->vertex_shader_module, m->geometry_shader_module);
		auto& rd = rvd.start(params, m->owner->get_matrix(), shader, *material, 0, m->topology, vb);

		rd.draw(params, m->vertex_buffer.get(), m->topology);
	}
	gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

void WorldUserMeshesEmitter::emit_transparent(const RenderParams& params, RenderViewData& rvd) {

}



