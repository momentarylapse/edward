#pragma once

#include <lib/base/pointer.h>
#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/target/TextureRenderer.h>

namespace yrenderer {

class MeshEmitter;
class SceneRenderer;
class CubeMapSource;


class CubeMapRenderer : public RenderTask {
public:
	explicit CubeMapRenderer(Context* ctx, SceneView& scene_view);

	CubeMapSource* source = nullptr;
	void set_source(CubeMapSource* s);
	owned<SceneRenderer> scene_renderers[6];
	owned<TextureRenderer> texture_renderers[6];

	void remove_all_emitters();
	void add_emitter(shared<MeshEmitter> emitter);

	void render(const RenderParams& params) override;
};

}
