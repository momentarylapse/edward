//
// Created by Michael Ankele on 2025-05-06.
//

#include "WorldTerrainsEmitter.h"
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <world/Terrain.h>
#include <y/ComponentManager.h>
#include <y/Entity.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>

WorldTerrainsEmitter::WorldTerrainsEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "ter") {}

void WorldTerrainsEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto& terrains = ComponentManager::get_list_family<Terrain>();
	for (auto *t: terrains) {
		auto o = t->owner;

		auto material = t->material.get();
		if (shadow_pass and !material->cast_shadow)
			return;
		auto shader = rvd.get_shader(material, 0, t->vertex_shader_module, "");
		if (shadow_pass)
			material = rvd.material_shadow;

		auto& rd = rvd.start(params, mat4::translation(o->pos), shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, t->vertex_buffer.get());

		if (!shadow_pass) {
#ifdef USING_VULKAN
			params.command_buffer->push_constant(0, 12, &t->texture_scale[0].x);
			params.command_buffer->push_constant(16, 12, &t->texture_scale[1].x);
#else
			shader->set_floats("pattern0", &t->texture_scale[0].x, 3);
			shader->set_floats("pattern1", &t->texture_scale[1].x, 3);
#endif
		}
		rd.draw_triangles(params, t->vertex_buffer.get());
	}
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

