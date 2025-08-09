/*
 * ShadowRenderer.h
 *
 *  Created on: Dec 11, 2022
 *      Author: michi
 */

#pragma once

#include <functional>

#include <lib/yrenderer/Renderer.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/math/mat4.h>
#include <lib/yrenderer/scene/SceneRenderer.h>
#include "../../scene/SceneView.h"

class Camera;
class Profiler;

namespace yrenderer {

class Material;
class MeshEmitter;
class GeometryEmitter;
class TextureRenderer;
struct SceneView;

class ShadowRenderer : public RenderTask {
public:
	explicit ShadowRenderer(Context* ctx, SceneView* parent, int resolution);

	static constexpr int NUM_CASCADES = 2;

	void add_emitter(shared<MeshEmitter> emitter);

    void render(const RenderParams& params) override;

    owned<Material> material;
	SceneView scene_view;
	SceneView* parent_scene;
	struct Cascade {
		Cascade();
		~Cascade();
		ygfx::DepthBuffer* depth_buffer = nullptr;
		owned<TextureRenderer> texture_renderer;
		float scale = 1.0f;
	    owned<SceneRenderer> scene_renderer;
	} cascades[NUM_CASCADES];

private:
    void render_cascade(const RenderParams& params, Cascade& c);
	void set_projection(const mat4& proj);
};

}

