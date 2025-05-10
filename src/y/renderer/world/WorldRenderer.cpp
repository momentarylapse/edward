/*
 * WorldRenderer.cpp
 *
 *  Created on: Jan 19, 2020
 *      Author: michi
 */

#include "WorldRenderer.h"
#include "../../graphics-impl.h"
#include "../../helper/PerformanceMonitor.h"
#include "../../fx/Particle.h"
#include "../../gui/Picture.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../world/Model.h"
#include "../../world/Terrain.h"
#include "../../world/World.h"
#include <lib/base/callable.h>
#include <renderer/helper/CubeMapSource.h>

#include "../../Config.h"
#include <y/ComponentManager.h>


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

WorldRenderer::WorldRenderer(const string &name, SceneView& _scene_view) :
		Renderer(name),
		scene_view(_scene_view)
{
}

void WorldRenderer::reset() {
}
