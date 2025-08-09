//
// Created by michi on 1/3/25.
//

#pragma once


#include <lib/yrenderer/Renderer.h>

class Camera;
class XTerrainVBUpdater;

namespace yrenderer {
	class CubeMapRenderer;
	class SceneRenderer;
	class HDRResolver;
	class PostProcessor;
	class TextureRenderer;
	class MultisampleResolver;
	class RenderPath;
	struct RenderViewData;
	class CubeMapSource;
	class LightMeter;
	enum class RenderPathType;
}

class FullCameraRenderer : public yrenderer::Renderer {
public:
	explicit FullCameraRenderer(yrenderer::Context* ctx, Camera* cam, yrenderer::RenderPathType type);
	~FullCameraRenderer() override;

	yrenderer::RenderPathType type;

	float shadow_box_size;
	int shadow_resolution;
	Camera* cam;

	yrenderer::RenderPath* render_path = nullptr;

	// post processing
	yrenderer::HDRResolver* hdr_resolver = nullptr;
	yrenderer::PostProcessor* post_processor = nullptr;
	yrenderer::TextureRenderer* texture_renderer = nullptr;
	yrenderer::MultisampleResolver* multisample_resolver = nullptr;
	yrenderer::LightMeter* light_meter = nullptr;

	Renderer* main_renderer = nullptr;


	void create_post_processing(Renderer* source);

	void prepare(const yrenderer::RenderParams& params) override;
	void draw(const yrenderer::RenderParams& params) override;

	//virtual void render_into_texture(FrameBuffer *fb, Camera *cam, RenderViewData &rvd) {};

	void render_cubemaps(const yrenderer::RenderParams& params);
	void prepare_instanced_matrices();

	yrenderer::CubeMapSource* cube_map_source = nullptr;
	void suggest_cube_map_pos();


	void check_terrains(const vec3& cam_pos);
	Array<XTerrainVBUpdater*> updater;
};

yrenderer::RenderPath* create_render_path(yrenderer::Context* ctx, yrenderer::RenderPathType type);
