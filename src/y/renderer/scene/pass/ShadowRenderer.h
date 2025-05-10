/*
 * ShadowRenderer.h
 *
 *  Created on: Dec 11, 2022
 *      Author: michi
 */

#pragma once

#include <functional>

#include "../../Renderer.h"
#include "../../../graphics-fwd.h"
#include <lib/math/mat4.h>
#include <renderer/scene/SceneRenderer.h>
#include "../../scene/SceneView.h"

class MeshEmitter;
class Camera;
class PerformanceMonitor;
class Material;
class GeometryEmitter;
class TextureRenderer;
struct SceneView;

class ShadowRenderer : public RenderTask {
public:
	explicit ShadowRenderer(SceneView* parent, shared_array<MeshEmitter> emitters);

	static constexpr int NUM_CASCADES = 2;

    void render(const RenderParams& params) override;

    owned<Material> material;
	SceneView scene_view;
	SceneView* parent_scene;
	struct Cascade {
		Cascade();
		~Cascade();
		DepthBuffer* depth_buffer = nullptr;
		owned<TextureRenderer> texture_renderer;
		float scale = 1.0f;
	    owned<SceneRenderer> scene_renderer;
	} cascades[NUM_CASCADES];

private:
    void render_cascade(const RenderParams& params, Cascade& c);
	void set_projection(const mat4& proj);
};

