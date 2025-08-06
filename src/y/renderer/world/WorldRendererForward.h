/*
 * WorldRendererGLForward.h
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#pragma once

#include "WorldRenderer.h"
#include <lib/yrenderer/scene/RenderViewData.h>

namespace yrenderer {
	class SceneRenderer;
}
class Camera;
class Profiler;

class WorldRendererForward : public WorldRenderer {
public:
	explicit WorldRendererForward(yrenderer::Context* ctx, Camera* cam, yrenderer::SceneView& scene_view);

	void prepare(const yrenderer::RenderParams& params) override;
	void draw(const yrenderer::RenderParams& params) override;

	yrenderer::SceneRenderer* scene_renderer;
};
