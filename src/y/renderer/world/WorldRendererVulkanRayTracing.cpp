/*
 * WorldRendererVulkanForward.cpp
 *
 *  Created on: Nov 18, 2021
 *      Author: michi
 */

#include "WorldRendererVulkanRayTracing.h"
#ifdef USING_VULKAN
#include "../helper/Raytracing.h"
#include "../scene/SceneView.h"
#include "../path/RenderPath.h"
#include "../base.h"
#include <graphics-impl.h>
#include <lib/os/msg.h>
#include <lib/profiler/Profiler.h>
#include "../../helper/ResourceManager.h"
#include "../../world/Camera.h"
#include "../../world/World.h"
#include "../../y/EngineData.h"
#include "../../Config.h"


WorldRendererVulkanRayTracing::WorldRendererVulkanRayTracing(vulkan::Device *_device, SceneView& scene_view, int w, int h) :
		WorldRenderer("rt", scene_view) {
	device = _device;
	width = w;
	height = h;

	rvd.set_scene_view(&scene_view);

	mode = RaytracingMode::NONE;
	if (device->has_rtx() and config.allow_rtx)
		mode = RaytracingMode::RTX;
	else if (device->has_compute())
		mode = RaytracingMode::COMPUTE;
	else
		throw Exception("neither RTX nor compute shader support");

	offscreen_image = new vulkan::StorageTexture(width, height, 1, "rgba:f16");
	offscreen_image->set_options("magfilter=nearest,minfilter=nearest");

	rt_setup_explicit(scene_view, mode);

	if (mode == RaytracingMode::RTX) {
		msg_error("RTX!!!");
		rtx.pool = new vulkan::DescriptorPool("acceleration-structure:1,image:1,storage-buffer:1,buffer:1024,sampler:1024", 1024);

		rtx.buffer_cam = new vulkan::UniformBuffer(sizeof(PushConst));

		rtx.dset = rtx.pool->create_set("acceleration-structure,image,buffer,buffer,buffer,buffer");
		rtx.dset->set_storage_image(1, offscreen_image);
		rtx.dset->set_uniform_buffer(2, rtx.buffer_cam);
		rtx.dset->set_uniform_buffer(4, rvd.ubo_light.get());
		rtx.dset->set_uniform_buffer(5, scene_view.ray_tracing_data->buffer_meshes.get());

		auto shader_gen = resource_manager->load_shader("vulkan/gen.shader");
		auto shader1 = resource_manager->load_shader("vulkan/group1.shader");
		auto shader2 = resource_manager->load_shader("vulkan/group2.shader");
		rtx.pipeline = new vulkan::RayPipeline("[[acceleration-structure,image,buffer,buffer,buffer,buffer]]", {shader_gen.get(), shader1.get(), shader2.get()}, 2);
		rtx.pipeline->create_sbt();


	} else if (mode == RaytracingMode::COMPUTE) {
		msg_error("COMPUTE!!!");

		compute.pool = new vulkan::DescriptorPool("image:1,storage-buffer:1,buffer:8,sampler:1", 1);

		auto shader = resource_manager->load_shader("compute/pathtracing.shader");
		compute.pipeline = new vulkan::ComputePipeline(shader.get());
		compute.dset = compute.pool->create_set("image,buffer,buffer");
		compute.dset->set_storage_image(0, offscreen_image);
		compute.dset->set_uniform_buffer(1, scene_view.ray_tracing_data->buffer_meshes.get());
		compute.dset->set_uniform_buffer(2, rvd.ubo_light.get());
		compute.dset->update();
	}

	pc.t_rand = 0;


	auto shader_out = resource_manager->load_shader("vulkan/passthrough.shader");
	out_renderer = new ThroughShaderRenderer("out", shader_out);
	out_renderer->bind_texture(0, offscreen_image);
}

void WorldRendererVulkanRayTracing::prepare(const RenderParams& params) {
	profiler::begin(ch_prepare);
	gpu_timestamp_begin(params, ch_prepare);

	rvd.set_view(params, scene_view.cam);
	rvd.update_light_ubo();

	int w = width * engine.resolution_scale_x;
	int h = height * engine.resolution_scale_y;

	pc.iview = scene_view.cam->view_matrix().inverse();
	pc.background = world.background;
	pc.num_lights = scene_view.lights.num;
	pc.t_rand += loop(pc.t_rand + 0.01f, 0.0f, 10.678f);

	auto cb = params.command_buffer;

	cb->image_barrier(offscreen_image,
		vulkan::AccessFlags::NONE, vulkan::AccessFlags::SHADER_WRITE_BIT,
		vulkan::ImageLayout::UNDEFINED, vulkan::ImageLayout::GENERAL);

	rt_update_frame(scene_view);

	if (mode == RaytracingMode::RTX) {

		rtx.buffer_cam->update(&pc);
		rtx.dset->set_acceleration_structure(0, scene_view.ray_tracing_data->rtx.tlas);
		rtx.dset->update();

		cb->set_bind_point(vulkan::PipelineBindPoint::RAY_TRACING);
		cb->bind_pipeline(rtx.pipeline);

		cb->bind_descriptor_set(0, rtx.dset);
		//cb->push_constant(0, sizeof(pc), &pc);

		cb->trace_rays(w, h, 1);
		
	} else if (mode == RaytracingMode::COMPUTE) {

		pc.num_trias = 0;
		pc.num_meshes = scene_view.ray_tracing_data->num_meshes;
		pc.num_lights = scene_view.lights.num;
		pc.out_width = w;
		pc.out_height = h;
		pc.out_ratio = engine.physical_aspect_ratio;


		cb->set_bind_point(vulkan::PipelineBindPoint::COMPUTE);
		cb->bind_pipeline(compute.pipeline);
		cb->bind_descriptor_set(0, compute.dset);
		cb->push_constant(0, sizeof(pc), &pc);
		const int GROUP_SIZE = 16;
		cb->dispatch(w / GROUP_SIZE, h / GROUP_SIZE, 1);
	}

	cb->set_bind_point(vulkan::PipelineBindPoint::GRAPHICS);

	/*cb->image_barrier(offscreen_image,
		vulkan::AccessFlags::SHADER_WRITE_BIT, vulkan::AccessFlags::SHADER_READ_BIT,
		vulkan::ImageLayout::GENERAL, vulkan::ImageLayout::SHADER_READ_ONLY_OPTIMAL);*/
	//cb->copy_image(offscreen_image, offscreen_image2, {0,0,w,h,0,0});

	cb->image_barrier(offscreen_image,
		vulkan::AccessFlags::SHADER_WRITE_BIT, vulkan::AccessFlags::SHADER_READ_BIT,
		vulkan::ImageLayout::GENERAL, vulkan::ImageLayout::SHADER_READ_ONLY_OPTIMAL);


	out_renderer->set_source(dynamicly_scaled_source());
	//out_renderer->bindings.shader_data.dict_set("model:0", mat4_to_any(mat4::ID));
	//out_renderer->bindings.shader_data.dict_set("view:64", mat4_to_any(mat4::ID));
	out_renderer->bindings.shader_data.dict_set("project:128", mat4_to_any(mat4::ID));
	out_renderer->bindings.shader_data.dict_set("scale_x:204", 1.0f);
	out_renderer->bindings.shader_data.dict_set("scale_y:208", 1.0f);

	gpu_timestamp_end(params, ch_prepare);
	profiler::end(ch_prepare);
}

void WorldRendererVulkanRayTracing::draw(const RenderParams& params) {
	out_renderer->draw(params);
}


#endif

