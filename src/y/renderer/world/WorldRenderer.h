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

#include "../scene/SceneView.h"

class GeometryRenderer;
class ShadowMapRenderer;
class Profiler;
class World;
class Camera;
struct mat4;
struct vec3;
struct quaternion;
class Material;
class CubeMapSource;



class WorldRenderer : public Renderer {
public:
	WorldRenderer(const string &name, SceneView& scene_view);

	bool wireframe = false;

	SceneView& scene_view;

	void reset();
};


