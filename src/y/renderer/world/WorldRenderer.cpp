/*
 * WorldRenderer.cpp
 *
 *  Created on: Jan 19, 2020
 *      Author: michi
 */

#include "WorldRenderer.h"
#include <lib/ygraphics/graphics-impl.h>
#include "../../fx/Particle.h"
#include "../../gui/Picture.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../world/Model.h"
#include "../../world/Terrain.h"
#include "../../world/World.h"
#include <lib/profiler/Profiler.h>
#include <lib/base/callable.h>
#include <lib/yrenderer/helper/CubeMapSource.h>

#include "../../Config.h"
#include <y/ComponentManager.h>


using namespace yrenderer;

struct GeoPush {
	alignas(16) mat4 model;
	alignas(16) color emission;
	alignas(16) vec3 eye_pos;
	alignas(16) float xxx[4];
};


mat4 mtr(const vec3 &t, const quaternion &a) {
	auto mt = mat4::translation(t);
	auto mr = mat4::rotation(a);
	return mt * mr;
}

WorldRenderer::WorldRenderer(Context* ctx, const string &name, Camera* _cam, SceneView& _scene_view) :
		Renderer(ctx, name),
		scene_view(_scene_view)
{
	cam = _cam;
}

void WorldRenderer::reset() {
}
