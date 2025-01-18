/*
 * WorldRenderer.h
 *
 *  Created on: Jan 19, 2020
 *      Author: michi
 */

#pragma once

#include "../Renderer.h"
#include "../../graphics-fwd.h"
#include <lib/math/mat4.h>
#include <lib/math/vec3.h>
#include <lib/image/color.h>
#include <lib/base/callable.h>
#include <lib/base/pointer.h>

#include "geometry/GeometryRenderer.h"
#include "geometry/SceneView.h"

class GeometryRenderer;
class ShadowMapRenderer;
class PerformanceMonitor;
class World;
class Camera;
class mat4;
class vec3;
class quaternion;
class Material;
class CubeMapSource;



class WorldRenderer : public Renderer {
public:
	WorldRenderer(const string &name, SceneView& scene_view);

	int ch_post = -1, ch_post_focus = -1;
	int ch_pre = -1, ch_bg = -1, ch_fx = -1, ch_world = -1, ch_prepare_lights = -1;

	bool wireframe = false;

	SceneView& scene_view;

	shared<Shader> shader_fx;
	owned<GeometryRenderer> geo_renderer;
//	virtual void render_into_texture(Camera *cam, RenderViewData &rvd, const RenderParams& params) = 0;

	void reset();
};


