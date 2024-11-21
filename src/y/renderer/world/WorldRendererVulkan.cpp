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
#include "../helper/CubeMapSource.h"
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

	// not sure this is a good idea...
	auto e = new Entity;
	cube_map_source = new CubeMapSource;
	cube_map_source->owner = e;
	cube_map_source->cube_map = new CubeMap(cube_map_source->resolution, "rgba:i8");

	scene_view.cube_map = cube_map_source->cube_map;

	if (false) {
		Image im;
		im.create(cube_map_source->resolution, cube_map_source->resolution, Red);
		scene_view.cube_map->write_side(0, im);
		im.create(cube_map_source->resolution, cube_map_source->resolution, color(1, 1,0.5f,0));
		scene_view.cube_map->write_side(1, im);
		im.create(cube_map_source->resolution, cube_map_source->resolution, color(1, 1,0,1));
		scene_view.cube_map->write_side(2, im);
	}

	vb_2d = new VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);

}

void WorldRendererVulkan::create_more() {
	shadow_renderer = new ShadowRendererVulkan();
	scene_view.fb_shadow1 = shadow_renderer->fb[0];
	scene_view.fb_shadow2 = shadow_renderer->fb[1];
	add_child(shadow_renderer.get());

	geo_renderer = new GeometryRendererVulkan(type, scene_view);
	add_child(geo_renderer.get());

}

WorldRendererVulkan::~WorldRendererVulkan() = default;


void WorldRendererVulkan::render_into_cubemap(CubeMapSource& source, const RenderParams& params) {
	if (!source.depth_buffer)
		source.depth_buffer = new DepthBuffer(source.resolution, source.resolution, "d:f32", true);
	if (!source.cube_map)
		source.cube_map = new CubeMap(source.resolution, "rgba:i8");
	if (!source.render_pass)
		source.render_pass = new vulkan::RenderPass({source.cube_map.get(), source.depth_buffer.get()}, {"autoclear"});
	if (!source.frame_buffer[0])
		for (int i=0; i<6; i++) {
			source.frame_buffer[i] = new FrameBuffer(source.render_pass.get(), {source.cube_map.get(), source.depth_buffer.get()});
			try {
				source.frame_buffer[i]->update_x(source.render_pass.get(), {source.cube_map.get(), source.depth_buffer.get()}, i);
			} catch(Exception &e) {
				msg_error(e.message());
				return;
			}
		}
	Entity o(source.owner->pos, quaternion::ID);
	Camera cube_cam;
	cube_cam.owner = &o;
	cube_cam.fov = pi/2;
	cube_cam.min_depth = source.min_depth;
	cube_cam.max_depth = source.max_depth;
	for (int i=0; i<6; i++) {
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
		auto sub_params = params.with_target(source.frame_buffer[i].get());
		sub_params.render_pass = source.render_pass.get();
		sub_params.desired_aspect_ratio = 1.0f;
		render_into_texture(&cube_cam, rvd_cube[i], sub_params);
	}
	cube_cam.owner = nullptr;
	//params.command_buffer->barrier({source.cube_map.get()}, 0);
}


void WorldRendererVulkan::prepare_lights(Camera *cam, RenderViewDataVK &rvd) {
	PerformanceMonitor::begin(ch_prepare_lights);

	scene_view.prepare_lights(shadow_box_size);
	rvd.ubo_light->update_part(&scene_view.lights[0], 0, scene_view.lights.num * sizeof(scene_view.lights[0]));
	PerformanceMonitor::end(ch_prepare_lights);
}

#endif



