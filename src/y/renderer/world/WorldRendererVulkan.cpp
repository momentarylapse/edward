/*
 * WorldRendererVulkan.cpp
 *
 *  Created on: Nov 18, 2021
 *      Author: michi
 */

#include "WorldRendererVulkan.h"
#ifdef USING_VULKAN
#include "pass/ShadowRendererVulkan.h"
#include "../../graphics-impl.h"
#include <lib/image/image.h>
#include <lib/math/vec3.h>
#include <lib/math/complex.h>
#include <lib/math/rect.h>
#include <lib/os/msg.h>
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../fx/Particle.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../y/Entity.h"



WorldRendererVulkan::WorldRendererVulkan(const string &name, Camera *cam, RenderPathType _type) : WorldRenderer(name, cam) {
	type = _type;

	vb_2d = nullptr;

	scene_view.cube_map = new CubeMap(cube_resolution, "rgba:i8");
	if (false) {
		Image im;
		im.create(cube_resolution, cube_resolution, Red);
		scene_view.cube_map->write_side(0, im);
		im.create(cube_resolution, cube_resolution, color(1, 1,0.5f,0));
		scene_view.cube_map->write_side(1, im);
		im.create(cube_resolution, cube_resolution, color(1, 1,0,1));
		scene_view.cube_map->write_side(2, im);
	}

	depth_cube = new DepthBuffer(cube_resolution, cube_resolution, "d:f32", true);

	render_pass_cube = new vulkan::RenderPass({scene_view.cube_map.get(), depth_cube.get()}, {"autoclear"});
	fb_cube = new vulkan::FrameBuffer(render_pass_cube, {scene_view.cube_map.get(), depth_cube.get()});



	vb_2d = new VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);



	resource_manager->load_shader_module("vulkan/module-surface-dummy.shader");
}

void WorldRendererVulkan::create_more() {
	shadow_renderer = new ShadowRendererVulkan();
	scene_view.fb_shadow1 = shadow_renderer->fb[0];
	scene_view.fb_shadow2 = shadow_renderer->fb[1];
	add_child(shadow_renderer.get());

	geo_renderer = new GeometryRendererVulkan(type, scene_view);
	add_child(geo_renderer.get());

}

WorldRendererVulkan::~WorldRendererVulkan() {
}


void WorldRendererVulkan::render_into_cubemap(const RenderParams& params, CubeMap *cube, const CubeMapParams &cube_params) {
	if (!fb_cube)
		fb_cube = new FrameBuffer(render_pass_cube, {depth_cube.get()});
	Entity o(cube_params.pos, quaternion::ID);
	Camera cube_cam;
	cube_cam.owner = &o;
	cube_cam.fov = pi/2;
	cube_cam.min_depth = cube_params.min_depth;
	for (int i=0; i<6; i++) {
		try {
			fb_cube->update_x(render_pass_cube, {cube, depth_cube.get()}, i);
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
		auto sub_params = params.with_target(fb_cube.get());
		sub_params.render_pass = render_pass_cube;
		render_into_texture(fb_cube.get(), &cube_cam, rvd_cube[i], sub_params);
	}
	cube_cam.owner = nullptr;
}


void WorldRendererVulkan::prepare_lights(Camera *cam, RenderViewDataVK &rvd) {
	PerformanceMonitor::begin(ch_prepare_lights);

	scene_view.prepare_lights(shadow_box_size);
	rvd.ubo_light->update_part(&scene_view.lights[0], 0, scene_view.lights.num * sizeof(scene_view.lights[0]));
	PerformanceMonitor::end(ch_prepare_lights);
}

#endif



