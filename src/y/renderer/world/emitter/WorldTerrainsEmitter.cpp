//
// Created by Michael Ankele on 2025-05-06.
//

#include "WorldTerrainsEmitter.h"
#include <lib/any/conversion.h>
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <world/Terrain.h>
#include <y/EntityManager.h>
#include <y/Entity.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/helper/Bindable.h>

WorldTerrainsEmitter::WorldTerrainsEmitter(yrenderer::Context* ctx) : MeshEmitter(ctx, "ter") {}

void WorldTerrainsEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	auto draw_terrains = [&params, &rvd, shadow_pass] (Terrain* t) {
		auto o = t->owner;

		auto material = t->material.get();
		if (shadow_pass and !material->cast_shadow)
			return;
		auto shader = rvd.get_shader(material, 0, t->vertex_shader_module, "");
		if (shadow_pass)
			material = rvd.material_shadow;

		auto& rd = rvd.start(params, mat4::translation(o->pos), shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, t->vertex_buffer.get());

		if (!shadow_pass) {
			Any data;
			data.dict_set("pattern0:0", vec3_to_any(t->texture_scale[0]));
			data.dict_set("pattern1:16", vec3_to_any(t->texture_scale[1]));
			yrenderer::apply_shader_data(params, shader, data);
		}
		rd.draw_triangles(params, t->vertex_buffer.get());
	};

	auto& terrains = EntityManager::global->get_component_list<Terrain>();
	for (auto *t: terrains) {
		draw_terrains(t);
	}

	auto& terrains2 = EntityManager::global->get_component_list<TerrainRef>();
	for (auto t: terrains2) {
		if (t->terrain) {
			t->terrain->owner = t->owner;
			draw_terrains(t->terrain);
		}
	}

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

