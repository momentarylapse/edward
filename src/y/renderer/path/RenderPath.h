//
// Created by michi on 1/3/25.
//

#ifndef RENDERPATH_H
#define RENDERPATH_H


#include <renderer/Renderer.h>
#include "../scene/SceneView.h"

class CubeMapRenderer;
class SceneRenderer;
class Camera;
class HDRResolver;
class PostProcessor;
class WorldRenderer;
class TextureRenderer;
class MultisampleResolver;
class LightMeter;
class GeometryRenderer;
class ShadowRenderer;
struct RenderViewData;
class CubeMapSource;

enum class RenderPathType {
	Direct,
	Forward,
	Deferred,
	PathTracing
};

class RenderPath : public Renderer {
public:
	explicit RenderPath(RenderPathType type, Camera* cam);
	~RenderPath() override;

	RenderPathType type = RenderPathType::Direct;

	float shadow_box_size;
	int shadow_resolution;
	Camera* cam;
	SceneView scene_view;

	WorldRenderer* world_renderer = nullptr;
	owned<ShadowRenderer> shadow_renderer;

	// post processing
	HDRResolver* hdr_resolver = nullptr;
	PostProcessor* post_processor = nullptr;
	TextureRenderer* texture_renderer = nullptr;
	MultisampleResolver* multisample_resolver = nullptr;
	LightMeter* light_meter = nullptr;

	Renderer* main_renderer = nullptr;


	void create_shadow_renderer();
	void create_cube_renderer();
	void create_post_processing(Renderer* source);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	//virtual void render_into_texture(FrameBuffer *fb, Camera *cam, RenderViewData &rvd) {};
	void render_into_cubemap(CubeMapSource& source);

	void prepare_basics();
	void render_cubemaps(const RenderParams& params);
	void prepare_instanced_matrices();

	CubeMapSource* cube_map_source = nullptr;
	owned<CubeMapRenderer> cube_map_renderer;
	void suggest_cube_map_pos();
};

WorldRenderer *create_world_renderer(SceneView& scene_view, RenderPathType type);

#endif //RENDERPATH_H
