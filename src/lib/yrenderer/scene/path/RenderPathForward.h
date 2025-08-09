/*
 * RenderPathForward.h
 *
 *  Created on: Jun 2, 2021
 *      Author: michi
 */

#pragma once

#include "RenderPath.h"

namespace yrenderer {
	class SceneRenderer;

class RenderPathForward : public RenderPath {
public:
	explicit RenderPathForward(Context* ctx, int shadow_resolution);

	void add_background_emitter(shared<MeshEmitter> emitter) override;
	void add_opaque_emitter(shared<MeshEmitter> emitter) override;
	void add_transparent_emitter(shared<MeshEmitter> emitter) override;

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	SceneRenderer* scene_renderer;
};

}
