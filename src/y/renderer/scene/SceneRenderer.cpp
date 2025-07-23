//
// Created by Michael Ankele on 2025-05-05.
//

#include "SceneRenderer.h"
#include "RenderViewData.h"
#include "SceneView.h"
#include <renderer/base.h>
#include <lib/profiler/Profiler.h>
#include <world/Light.h>
#include <graphics-impl.h>
#include "renderer/helper/PipelineManager.h"


#ifdef USING_VULKAN
GraphicsPipeline* SceneRenderer::get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb) {
	if (pass.mode == TransparencyMode::FUNCTIONS)
		return PipelineManager::get_alpha(s, rp, top, vb, pass.source, pass.destination, pass.cull_mode, pass.z_test, pass.z_buffer);
	if (pass.mode == TransparencyMode::COLOR_KEY_HARD)
		return PipelineManager::get_alpha(s, rp, top, vb, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, pass.cull_mode, pass.z_test, pass.z_buffer);
	return PipelineManager::get(s, rp, top, vb, pass.cull_mode, pass.z_test, pass.z_buffer);
}
#endif


SceneRenderer::SceneRenderer(RenderPathType type, SceneView& _scene_view) : Renderer("scene"), scene_view(_scene_view) {
	rvd.set_scene_view(&scene_view);
	rvd.type = type;
}

SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::add_emitter(shared<MeshEmitter> emitter) {
	emitters.add(emitter);
}

void SceneRenderer::set_view(const RenderParams& params, const vec3& pos, const quaternion& ang, const mat4& proj) {
	rvd.set_view(params, pos, ang, proj);
}

void SceneRenderer::set_view_from_camera(const RenderParams& params, Camera* cam) {
	rvd.set_view(params, cam);
}

void SceneRenderer::prepare(const RenderParams& params) {
	profiler::begin(ch_prepare);

	if (!is_shadow_pass)
		rvd.update_light_ubo();
	profiler::end(ch_prepare);
}

void SceneRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	gpu_timestamp_begin(params, channel);
	rvd.begin_draw();

	if (background_color)
		rvd.clear(params, {*background_color}, 1.0f);

#ifdef USING_OPENGL
	bool flip_y = params.target_is_window;
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);
	//cur_rvd.set_wire(wireframe);
#endif

	if (allow_opaque)
		for (auto e: weak(emitters))
			e->emit(params, rvd, is_shadow_pass);

	if (allow_transparent and !is_shadow_pass)
		for (auto e: weak(emitters))
			e->emit_transparent(params, rvd);

#ifdef USING_OPENGL
	nix::set_front(nix::Orientation::CW);
#endif

	gpu_timestamp_end(params, channel);
	profiler::end(channel);
}




