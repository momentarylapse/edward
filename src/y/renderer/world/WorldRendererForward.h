/*
 * WorldRendererGLForward.h
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#pragma once

#include "WorldRenderer.h"
#include "geometry/RenderViewData.h"

class Camera;
class PerformanceMonitor;

class WorldRendererForward : public WorldRenderer {
public:
	WorldRendererForward(SceneView& scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void draw_with(const RenderParams& params);
	//void render_into_texture(Camera *cam, RenderViewData &rvd, const RenderParams& params) override;
};
