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
#include "../geometry/SceneView.h"

class Camera;
class PerformanceMonitor;
class Material;
class GeometryEmitter;
class TextureRenderer;
struct SceneView;

class ShadowRendererX : public RenderTask {
public:
	explicit ShadowRendererX(Camera* cam, std::function<GeometryEmitter*(SceneView&)> factory);

	static constexpr int NUM_CASCADES = 2;

	void prepare(const RenderParams& params) override {};
	void draw(const RenderParams& params) override {}

	void set_projection(const mat4& proj);
    void render(const RenderParams& params) override;

    owned<Material> material;
	SceneView scene_view;
	struct Cascade {
		Cascade();
		~Cascade();
		DepthBuffer* depth_buffer = nullptr;
		owned<TextureRenderer> texture_renderer;
		float scale = 1.0f;
	    owned<GeometryEmitter> geo_renderer;
	} cascades[NUM_CASCADES];


    void render_cascade(const RenderParams& params, Cascade& c);
};

