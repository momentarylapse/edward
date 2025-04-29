//
// Created by michi on 1/3/25.
//

#include "RenderPathDirect.h"
#include "../world/WorldRenderer.h"
#include "../world/pass/ShadowRenderer.h"
#include "../world/geometry/SceneView.h"
#include "../../world/Camera.h"


RenderPathDirect::RenderPathDirect(Camera* cam) : RenderPath(RenderPathType::Direct, cam) {
	world_renderer = create_world_renderer(scene_view, RenderPathType::Forward);
	geo_renderer = world_renderer->geo_renderer.get();
	create_shadow_renderer();
}

void RenderPathDirect::prepare(const RenderParams &params) {
	prepare_basics();
	scene_view.choose_lights();

	scene_view.cam->update_matrices(params.desired_aspect_ratio);
	geo_renderer->cur_rvd.set_projection_matrix(scene_view.cam->m_projection);
	geo_renderer->cur_rvd.set_view_matrix(scene_view.cam->m_view);
	geo_renderer->cur_rvd.update_lights();

	for (int i: scene_view.shadow_indices) {
		shadow_renderer->set_projection(scene_view.lights[i]->shadow_projection);
		shadow_renderer->render(params);
	}
	world_renderer->prepare(params);
}

void RenderPathDirect::draw(const RenderParams &params) {
	world_renderer->draw(params);
}

