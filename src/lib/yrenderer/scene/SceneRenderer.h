//
// Created by Michael Ankele on 2025-05-05.
//

#pragma once

#include <lib/yrenderer/Renderer.h>
#include "MeshEmitter.h"
#include "RenderViewData.h"

namespace yrenderer {

class MeshEmitter;

enum class RenderPathType {
	Direct,
	Forward,
	Deferred,
	PathTracing
};

class SceneRenderer : public Renderer {
public:
	explicit SceneRenderer(Context* ctx, RenderPathType type, SceneView& scene_view);
	~SceneRenderer() override;

	shared_array<MeshEmitter> emitters;
	void add_emitter(shared<MeshEmitter> emitter);

	void set_view(const RenderParams& params, const CameraParams& cam, const mat4* proj = nullptr);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	SceneView& scene_view;
	RenderViewData rvd;

	base::optional<color> background_color;
	bool is_shadow_pass = false;

	static constexpr bool using_view_space = true;

#ifdef USING_VULKAN
	static ygfx::GraphicsPipeline* get_pipeline(ygfx::Shader *s, ygfx::RenderPass *rp, const Material::RenderPassData &pass, ygfx::PrimitiveTopology top, ygfx::VertexBuffer *vb);
#endif
};

}


