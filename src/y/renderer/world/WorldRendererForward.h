/*
 * WorldRendererGLForward.h
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#pragma once

#include "WorldRenderer.h"
#include "../scene/RenderViewData.h"

class SceneRenderer;
class Camera;
class Profiler;

class WorldRendererForward : public WorldRenderer {
public:
	explicit WorldRendererForward(SceneView& scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	SceneRenderer* scene_renderer;
};
