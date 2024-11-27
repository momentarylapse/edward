/*
 * WorldRendererGLForward.h
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#pragma once

#include "WorldRendererGL.h"
#ifdef USING_OPENGL
#include "geometry/RenderViewData.h"

class Camera;
class PerformanceMonitor;

class WorldRendererGLForward : public WorldRendererGL {
public:
	WorldRendererGLForward(Camera *cam);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void draw_with(const RenderParams& params, RenderViewData& rvd);

	void render_into_texture(FrameBuffer *fb, Camera *cam, RenderViewData &rvd) override;

	RenderViewData main_rvd;
};

#endif
