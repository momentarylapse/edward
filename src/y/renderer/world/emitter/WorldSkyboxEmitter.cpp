//
// Created by Michael Ankele on 2025-05-07.
//

#include "WorldSkyboxEmitter.h"
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <lib/yrenderer/Context.h>
#include <lib/profiler/Profiler.h>
#include <world/World.h>
#include <world/Model.h>
#include <world/components/Camera.h>
#include <lib/ygraphics/graphics-impl.h>

#include "lib/os/msg.h"

using namespace yrenderer;

WorldSkyboxEmitter::WorldSkyboxEmitter(Context* ctx) : MeshEmitter(ctx, "sky") {
}

void WorldSkyboxEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	if (shadow_pass)
		return;

	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	rvd.clear(params, {world->background}, 1.0f);

	float max_radius = -1;
	for (auto sb: weak(world->skybox))
		if (auto m = sb->model)
			max_radius = max(max_radius, m->prop.radius);
	if (max_radius < 0)
		max_radius = 1;

#if 1

#ifdef USING_OPENGL
	nix::set_cull(nix::CullMode::NONE);
#endif

	/*float max_depth = cam->max_depth;
	float min_depth = cam->min_depth;
	cam->min_depth = 0.1f;
	cam->max_depth = 2000000;
	auto pp = cam->projection_matrix(params.desired_aspect_ratio);*/

	// overwrite rendering parameters
	auto mv = rvd.ubo.v;
	auto mp = rvd.ubo.p;
	//rvd.set_view(params, {0,0,0}, rvd.view_ang, mat4::scale(1,1,0.1f) * pp); // :P
	const float scale = min(rvd.camera_params.max_depth / max_radius * 0.1f, 1.0f);
	rvd.ubo.v = mat4::rotation(rvd.camera_params.ang.bar());
	rvd.ubo.p = mp * mat4::scale(scale, scale, scale);

	// not working anymore... should have 2nd light data ubo
	int nlights = rvd.light_meta_data.num_lights;
	rvd.light_meta_data.num_lights = 0;

	for (auto sb: weak(world->skybox))
		if (auto m = sb->model) {
			sb->update_materials();
			auto matrix = mat4::rotation(sb->owner->ang);

			for (int i=0; i<m->materials.num; i++) {
				auto vb = m->mesh[0]->sub[i].vertex_buffer;
				auto shader = rvd.get_shader(sb->materials[i], 0, "default", "");
				auto& rd = rvd.start(params, matrix, shader, sb->materials[i], 0, ygfx::PrimitiveTopology::TRIANGLES, vb);

				rd.draw_triangles(params, vb);
			}
		}

	rvd.ubo.v = mv;
	rvd.ubo.p = mp;
	/*rvd.light_meta_data.num_lights = nlights;
	cam->min_depth = min_depth;
	cam->max_depth = max_depth;*/
#ifdef USING_OPENGL
	nix::set_cull(nix::CullMode::BACK);
#endif
#endif
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


