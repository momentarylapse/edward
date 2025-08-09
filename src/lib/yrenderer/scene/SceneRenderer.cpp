//
// Created by Michael Ankele on 2025-05-05.
//

#include "SceneRenderer.h"
#include "RenderViewData.h"
#include "SceneView.h"
#include <lib/yrenderer/Context.h>
#include <lib/profiler/Profiler.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/helper/PipelineManager.h>

namespace yrenderer {

	using namespace ygfx;

#ifdef USING_VULKAN
GraphicsPipeline* SceneRenderer::get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb) {
	if (pass.mode == TransparencyMode::FUNCTIONS)
		return PipelineManager::get_alpha(s, rp, top, vb, pass.source, pass.destination, pass.cull_mode, pass.z_test, pass.z_buffer);
	if (pass.mode == TransparencyMode::COLOR_KEY_HARD)
		return PipelineManager::get_alpha(s, rp, top, vb, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, pass.cull_mode, pass.z_test, pass.z_buffer);
	return PipelineManager::get(s, rp, top, vb, pass.cull_mode, pass.z_test, pass.z_buffer);
}
#endif


SceneRenderer::SceneRenderer(Context* ctx, RenderPathType type, SceneView& _scene_view) : Renderer(ctx, "scene"), scene_view(_scene_view), rvd(ctx) {
	rvd.set_scene_view(&scene_view);
	rvd.type = type;
}

SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::add_emitter(shared<MeshEmitter> emitter) {
	emitters.add(emitter);
}

void SceneRenderer::set_view(const RenderParams& params, const CameraParams& cam, const mat4* proj) {
	rvd.set_view(params, cam, proj);
}

void SceneRenderer::prepare(const RenderParams& params) {
	profiler::begin(ch_prepare);

	if (!is_shadow_pass)
		rvd.update_light_ubo();
	profiler::end(ch_prepare);
}

void SceneRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);
	rvd.begin_draw();

	if (background_color)
		rvd.clear(params, {*background_color}, 1.0f);

#ifdef USING_OPENGL
	bool flip_y = params.target_is_window;
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);
	//cur_rvd.set_wire(wireframe);
#endif

	for (auto e: weak(emitters))
		e->emit(params, rvd, is_shadow_pass);

	/*if (allow_transparent and !is_shadow_pass)
		for (auto e: weak(emitters))
			e->emit_transparent(params, rvd);*/

#ifdef USING_OPENGL
	nix::set_front(nix::Orientation::CW);
#endif

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

}




