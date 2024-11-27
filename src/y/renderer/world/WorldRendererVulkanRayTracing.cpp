/*
 * WorldRendererVulkanForward.cpp
 *
 *  Created on: Nov 18, 2021
 *      Author: michi
 */

#include "WorldRendererVulkanRayTracing.h"
#ifdef USING_VULKAN
#include "../../graphics-impl.h"
#include "../base.h"
#include "../../lib/os/msg.h"
#include "../../lib/base/iter.h"
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include "../../gui/Node.h"
#include "../../fx/Particle.h"
#include "../../world/Camera.h"
#include "../../world/Light.h"
#include "../../world/Material.h"
#include "../../world/Model.h"
#include "../../world/Terrain.h"
#include "../../world/World.h"
#include "../../y/ComponentManager.h"
#include "../../y/Entity.h"
#include "../../y/EngineData.h"
#include "../../Config.h"
#include "../../meta.h"

static const int MAX_RT_TRIAS = 65536;
static const int MAX_RT_MESHES = 1024;

WorldRendererVulkanRayTracing::WorldRendererVulkanRayTracing(vulkan::Device *_device, Camera *cam, int w, int h) :
		WorldRendererVulkan("rt", cam, RenderPathType::FORWARD) {
	device = _device;
	width = w;
	height = h;

	//create_more();
	geo_renderer = new GeometryRendererVulkan(type, scene_view);

	if (device->has_rtx() and config.allow_rtx)
		mode = Mode::RTX;
	else if (device->has_compute())
		mode = Mode::COMPUTE;
	else
		throw Exception("neither RTX nor compute shader support");

	offscreen_image = new vulkan::StorageTexture(width, height, 1, "rgba:f16");
	offscreen_image2 = new vulkan::Texture(width, height, "rgba:f16");

	buffer_meshes = new vulkan::UniformBuffer(sizeof(MeshDescription) * MAX_RT_MESHES);

	if (mode == Mode::RTX) {
		msg_error("RTX!!!");
		rtx.pool = new vulkan::DescriptorPool("acceleration-structure:1,image:1,storage-buffer:1,buffer:1024,sampler:1024", 1024);

		rtx.buffer_cam = new vulkan::UniformBuffer(sizeof(PushConst));

		rtx.dset = rtx.pool->create_set("acceleration-structure,image,buffer,buffer,buffer,buffer");
		rtx.dset->set_storage_image(1, offscreen_image);
		rtx.dset->set_uniform_buffer(2, rtx.buffer_cam);
		rtx.dset->set_uniform_buffer(4, rvd.ubo_light.get());
		rtx.dset->set_uniform_buffer(5, buffer_meshes);

		auto shader_gen = resource_manager->load_shader("vulkan/gen.shader");
		auto shader1 = resource_manager->load_shader("vulkan/group1.shader");
		auto shader2 = resource_manager->load_shader("vulkan/group2.shader");
		rtx.pipeline = new vulkan::RayPipeline("[[acceleration-structure,image,buffer,buffer,buffer,buffer]]", {shader_gen.get(), shader1.get(), shader2.get()}, 2);
		rtx.pipeline->create_sbt();


	} else if (mode == Mode::COMPUTE) {

		compute.pool = new vulkan::DescriptorPool("image:1,storage-buffer:1,buffer:8,sampler:1", 1);

		auto shader = resource_manager->load_shader("compute/pathtracing.shader");
		compute.pipeline = new vulkan::ComputePipeline(shader.get());
		compute.dset = compute.pool->create_set("image,buffer,buffer");
		compute.dset->set_storage_image(0, offscreen_image);
		compute.dset->set_uniform_buffer(1, buffer_meshes);
		compute.dset->set_uniform_buffer(2, rvd.ubo_light.get());
		compute.dset->update();
	}



	shader_out = resource_manager->load_shader("vulkan/passthrough.shader");
	dset_out = pool->create_set("sampler");

	dset_out->set_texture(0, offscreen_image2);
	dset_out->update();

	vb_2d = new VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);


	dummy_cam_entity = new Entity;
	dummy_cam = new Camera;
	dummy_cam_entity->components.add(dummy_cam);
	dummy_cam->owner = dummy_cam_entity;
}

static int cur_query_offset;

void WorldRendererVulkanRayTracing::prepare(const RenderParams& params) {
	if (!scene_view.cam)
		scene_view.cam = cam_main;

	scene_view.check_terrains(cam_main->owner->pos);
	prepare_lights(dummy_cam, rvd);

	int w = width * engine.resolution_scale_x;
	int h = height * engine.resolution_scale_y;

	pc.iview = scene_view.cam->view_matrix().inverse();
	pc.background = world.background;
	pc.num_lights = scene_view.lights.num;

	auto cb = params.command_buffer;

	cb->image_barrier(offscreen_image,
		vulkan::AccessFlags::NONE, vulkan::AccessFlags::SHADER_WRITE_BIT,
		vulkan::ImageLayout::UNDEFINED, vulkan::ImageLayout::GENERAL);

	auto& models = ComponentManager::get_list_family<Model>();
	auto& terrains = ComponentManager::get_list_family<Terrain>();


	Array<MeshDescription> meshes;

	for (auto m: models) {
		m->update_matrix();
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];

			MeshDescription md;
			md.matrix = m->_matrix;
			md.num_triangles = m->mesh[0]->sub[i].triangle_index.num / 3;
			md.albedo = material->albedo.with_alpha(material->roughness);
			md.emission = material->emission.with_alpha(material->metal);
			md.address_vertices = m->mesh[0]->sub[i].vertex_buffer->vertex_buffer.get_device_address();
			//md.address_indices = m->mesh[0]->sub[i].vertex_buffer->index_buffer.get_device_address();
			meshes.add(md);
		}
	}
	for (auto *t: terrains) {
		auto o = t->owner;

		MeshDescription md;
		md.matrix = mat4::translation(o->pos);
		md.albedo = t->material->albedo.with_alpha(t->material->roughness);
		md.emission = t->material->emission.with_alpha(t->material->metal);
		md.num_triangles = t->vertex_buffer->output_count / 3;
		md.address_vertices = t->vertex_buffer->vertex_buffer.get_device_address();
		meshes.add(md);
	}


	buffer_meshes->update_array(meshes, 0);

	if (mode == Mode::RTX) {

		Array<mat4> matrices;

		if (rtx.tlas) {
			// update
			for (auto m: models) {
				m->update_matrix();
				for (int i=0; i<m->material.num; i++)
					matrices.add(m->owner->get_matrix().transpose());
			}
			for (auto *t: terrains) {
				auto o = t->owner;
				matrices.add(mat4::translation(o->pos).transpose());
			}
			rtx.tlas->update_top(rtx.blas, matrices);

		} else {

			auto make_indexed = [] (VertexBuffer *vb) {
				if (!vb->is_indexed()) {
					Array<int> index;
					for (int i=0; i<vb->output_count; i++)
						index.add(i);
					vb->update_index(index);
				}
			};

			for (auto m: models) {
				m->update_matrix();
				for (int i=0; i<m->material.num; i++) {
					m->update_matrix();
					auto vb = m->mesh[0]->sub[i].vertex_buffer;
					make_indexed(vb);
					rtx.blas.add(vulkan::AccelerationStructure::create_bottom(device, vb));
					matrices.add(m->owner->get_matrix().transpose());
				}
			}

			for (auto *t: terrains) {
				auto o = t->owner;
				make_indexed(t->vertex_buffer.get());
				rtx.blas.add(vulkan::AccelerationStructure::create_bottom(device, t->vertex_buffer.get()));
				matrices.add(mat4::translation(o->pos).transpose());
			}

			rtx.tlas = vulkan::AccelerationStructure::create_top(device, rtx.blas, matrices);
		}

		rtx.buffer_cam->update(&pc);
		rtx.dset->set_acceleration_structure(0, rtx.tlas);
		rtx.dset->update();
		
		cb->set_bind_point(vulkan::PipelineBindPoint::RAY_TRACING);
		cb->bind_pipeline(rtx.pipeline);

		cb->bind_descriptor_set(0, rtx.dset);
		//cb->push_constant(0, sizeof(pc), &pc);

		cb->trace_rays(w, h, 1);
		
	} else if (mode == Mode::COMPUTE) {

		pc.num_trias = 0;
		pc.num_meshes = meshes.num;
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
	cb->copy_image(offscreen_image, offscreen_image2, {0,0,w,h,0,0});

	cb->image_barrier(offscreen_image,
		vulkan::AccessFlags::SHADER_WRITE_BIT, vulkan::AccessFlags::SHADER_READ_BIT,
		vulkan::ImageLayout::GENERAL, vulkan::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

}

void WorldRendererVulkanRayTracing::draw(const RenderParams& params) {

	auto cb = params.command_buffer;

    //vb_2d->create_quad(rect::ID_SYM, rect::ID);//dynamicly_scaled_source());
	vb_2d->create_quad(rect::ID_SYM, dynamicly_scaled_source());


	if (!pipeline_out) {
		pipeline_out = new vulkan::GraphicsPipeline(shader_out.get(), params.render_pass, 0, "triangles", "3f,3f,2f");
		pipeline_out->set_culling(CullMode::NONE);
		pipeline_out->set_z(false, false);
		pipeline_out->rebuild();
	}

	cb->bind_pipeline(pipeline_out);
	cb->bind_descriptor_set(0, dset_out);
	struct PCOut {
		mat4 p, m, v;
		float x[32];
	};
	PCOut pco = {mat4::ID, mat4::ID, mat4::ID, scene_view.cam->exposure};
    pco.x[3] = 1; // scale_x
    pco.x[4] = 1;
	cb->push_constant(0, sizeof(mat4) * 3 + 5 * sizeof(float), &pco);
	cb->draw(vb_2d.get());
}

void WorldRendererVulkanRayTracing::render_into_texture(Camera *cam, RenderViewData &rvd, const RenderParams& params) {
}

#endif

