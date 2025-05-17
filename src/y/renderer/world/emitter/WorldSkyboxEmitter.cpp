//
// Created by Michael Ankele on 2025-05-07.
//

#include "WorldSkyboxEmitter.h"
#include "../../scene/RenderViewData.h"
#include "../../scene/SceneView.h"
#include "../../base.h"
#include <helper/PerformanceMonitor.h>
#include <world/World.h>
#include <world/Model.h>
#include <world/Camera.h>
#include <graphics-impl.h>

WorldSkyboxEmitter::WorldSkyboxEmitter() : MeshEmitter("sky") {
}

void WorldSkyboxEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	if (shadow_pass)
		return;

	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);

	rvd.clear(params, {world.background}, 1.0f);

#if 1

	auto cam = rvd.scene_view->cam;
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
	rvd.ubo.v = mat4::rotation(rvd.view_ang.bar());
	rvd.ubo.p = mp * mat4::scale(0.01f, 0.01f, 0.01f);

	// not working anymore... should have 2nd light data ubo
	int nlights = rvd.light_meta_data.num_lights;
	rvd.light_meta_data.num_lights = 0;

	for (auto *sb: world.skybox) {
		sb->_matrix = mat4::rotation(sb->owner->ang);

		for (int i=0; i<sb->material.num; i++) {
			auto vb = sb->mesh[0]->sub[i].vertex_buffer;
			auto shader = rvd.get_shader(sb->material[i], 0, "default", "");
			auto& rd = rvd.start(params, sb->_matrix * mat4::scale(10,10,10), shader, *sb->material[i], 0, PrimitiveTopology::TRIANGLES, vb);

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
	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}


