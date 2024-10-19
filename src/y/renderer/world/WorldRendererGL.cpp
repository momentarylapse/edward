/*
 * WorldRendererGL.cpp
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#include "WorldRendererGL.h"

#ifdef USING_OPENGL
#include "geometry/GeometryRendererGL.h"
#include "pass/ShadowRendererGL.h"
#include "../base.h"
#include <graphics-impl.h>
#include <world/World.h>
#include <world/Light.h>
#include <world/Camera.h>
#include <helper/PerformanceMonitor.h>
#include <y/ComponentManager.h>
#include <lib/os/msg.h>


namespace nix {
	void resolve_multisampling(FrameBuffer *target, FrameBuffer *source);
}
void apply_shader_data(Shader *s, const Any &shader_data);


WorldRendererGL::WorldRendererGL(const string &name, Camera *cam, RenderPathType _type) :
		WorldRenderer(name, cam) {
	type = _type;

	depth_cube = new nix::DepthBuffer(cube_resolution, cube_resolution, "d24s8");
	fb_cube = nullptr;
	scene_view.cube_map = new nix::CubeMap(cube_resolution, "rgba:i8");
	scene_view.ubo_light = new nix::UniformBuffer();
}

void WorldRendererGL::create_more() {
	shadow_renderer = new ShadowRendererGL();
	scene_view.fb_shadow1 = shadow_renderer->fb[0];
	scene_view.fb_shadow2 = shadow_renderer->fb[1];
	add_child(shadow_renderer.get());

	geo_renderer = new GeometryRendererGL(type, scene_view);
	add_child(geo_renderer.get());
}

void WorldRendererGL::prepare_lights() {
	PerformanceMonitor::begin(ch_prepare_lights);
	scene_view.prepare_lights(shadow_box_size);
	PerformanceMonitor::end(ch_prepare_lights);
}

void WorldRendererGL::render_into_cubemap(DepthBuffer *depth, CubeMap *cube, const CubeMapParams &params) {
	if (!fb_cube)
		fb_cube = new nix::FrameBuffer({depth});
	Entity o(params.pos, quaternion::ID);
	Camera cam;
	cam.min_depth = params.min_depth;
	cam.owner = &o;
	cam.fov = pi/2;
	for (int i=0; i<6; i++) {
		try {
			fb_cube->update_x({cube, depth}, i);
		} catch(Exception &e) {
			msg_error(e.message());
			return;
		}
		if (i == 0)
			o.ang = quaternion::rotation(vec3(0,pi/2,0));
		if (i == 1)
			o.ang = quaternion::rotation(vec3(0,-pi/2,0));
		if (i == 2)
			o.ang = quaternion::rotation(vec3(-pi/2,pi,pi));
		if (i == 3)
			o.ang = quaternion::rotation(vec3(pi/2,pi,pi));
		if (i == 4)
			o.ang = quaternion::rotation(vec3(0,0,0));
		if (i == 5)
			o.ang = quaternion::rotation(vec3(0,pi,0));
		//prepare_lights(&cam);
		render_into_texture(fb_cube.get(), &cam);
	}
	cam.owner = nullptr;
}




#endif
