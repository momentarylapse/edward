#pragma once

#include <lib/base/pointer.h>
#include <lib/base/map.h>
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

class MultiCubeMapRenderer : public RenderTask {
public:
	explicit MultiCubeMapRenderer(Context* ctx, SceneView& scene_view);
	~MultiCubeMapRenderer() override;

	void add_emitter(shared<MeshEmitter> emitter);
	void remove_all_emitters();

	void render(const RenderParams& params) override;
	void render_into_cubemap(const RenderParams& params, CubeMapSource& source);

	SceneView& scene_view;
	Array<CubeMapSource*> sources; // input...
	shared_array<MeshEmitter> emitters;

	//base::map<CubeMapSource*, owned<CubeMapRenderer>> renderers;
	base::map<CubeMapSource*, CubeMapRenderer*> renderers;
};

}
