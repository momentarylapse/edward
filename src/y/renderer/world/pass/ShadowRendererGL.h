/*
 * ShadowRendererGL.h
 *
 *  Created on: Dec 11, 2022
 *      Author: michi
 */

#pragma once

#include "../../Renderer.h"
#include "../../../graphics-fwd.h"
#ifdef USING_OPENGL
#include <lib/math/mat4.h>
#include "../geometry/SceneView.h"

class Camera;
class PerformanceMonitor;
class Material;
class GeometryRendererGL;
struct SceneView;

class ShadowRendererGL : public Renderer {
public:
	ShadowRendererGL();

	shared<FrameBuffer> fb[2];

    void render_shadow_map(FrameBuffer *sfb, float scale);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override {}

    void render(SceneView &parent_scene_view);

    owned<Material> material;
	mat4 proj;
	SceneView scene_view;

    owned<GeometryRendererGL> geo_renderer;
};

#endif
