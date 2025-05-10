//
// Created by Michael Ankele on 2025-05-05.
//

#pragma once

#include "../Renderer.h"
#include "MeshEmitter.h"
#include "RenderViewData.h"

class Camera;
class MeshEmitter;
enum class RenderPathType;

class SceneRenderer : public Renderer {
public:
	explicit SceneRenderer(RenderPathType type, SceneView& scene_view);
	~SceneRenderer() override;

	shared_array<MeshEmitter> emitters;
	void add_emitter(shared<MeshEmitter> emitter);

	void set_view(const RenderParams& params, const vec3& pos, const quaternion& ang, const mat4& proj);
	void set_view_from_camera(const RenderParams& params, Camera* cam);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	SceneView& scene_view;
	RenderViewData rvd;

	base::optional<color> background_color;
	bool is_shadow_pass = false;
	bool allow_opaque = true;
	bool allow_transparent = true;

	static constexpr bool using_view_space = true;

#ifdef USING_VULKAN
	static GraphicsPipeline* get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb);
#endif
};


