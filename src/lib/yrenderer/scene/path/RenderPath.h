/*
 * RenderPath.h
 *
 *  Created on: Jan 19, 2020
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <lib/yrenderer/scene/MeshEmitter.h>
#include <lib/base/pointer.h>
#include <lib/image/color.h>

namespace yrenderer {

class ShadowMapRenderer;
class MeshEmitter;
class ShadowRenderer;
class CubeMapRenderer;
class CubeMapSource;



class RenderPath : public Renderer {
public:
	RenderPath(Context* ctx, const string &name);
	~RenderPath() override;

	CameraParams view;
	bool wireframe = false;
	float ambient_occlusion_radius = 0;
	base::optional<color> background_color;

	SceneView scene_view;

	owned<ShadowRenderer> shadow_renderer;
	owned<CubeMapRenderer> cube_map_renderer;

	virtual void remove_all_emitters() {}
	virtual void add_background_emitter(shared<MeshEmitter> emitter) {}
	virtual void add_opaque_emitter(shared<MeshEmitter> emitter) {}
	virtual void add_transparent_emitter(shared<MeshEmitter> emitter) {}


	void create_shadow_renderer(int resolution);
	void create_cube_renderer();

	// call BEFORE prepare()!
	void set_view(const CameraParams& view);
	void set_lights(const Array<Light*>& lights);

	void reset();

	void render_into_cubemap(const RenderParams& params, CubeMapSource& source);

	// shader modules
	static string light_sources_module;
	static string shadow_method;
	static string lighting_method;
};

}
