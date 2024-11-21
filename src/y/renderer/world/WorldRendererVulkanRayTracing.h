/*
 * WorldRendererVulkanRayTracing.h
 *
 *  Created on: Sep 23, 2022
 *      Author: michi
 */

#pragma once

#include "WorldRendererVulkan.h"
#ifdef USING_VULKAN

class Camera;

class WorldRendererVulkanRayTracing : public WorldRendererVulkan {
public:
	WorldRendererVulkanRayTracing(vulkan::Device *device, Camera *cam, int width, int height);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void render_into_texture(Camera *cam, RenderViewDataVK &rvd, const RenderParams& params) override;

	enum class Mode {
		NONE,
		COMPUTE,
		RTX
	} mode = Mode::NONE;

	vulkan::Device *device;
	RenderViewDataVK rvd;

	vulkan::StorageTexture *offscreen_image;
	vulkan::Texture *offscreen_image2;
	int width, height;

	struct MeshDescription {
		mat4 matrix;
		color albedo;
		color emission;
		int64 address_vertices;
		int64 address_indices;
		int num_triangles;
		int _a, _b, _c;
	};

	struct PushConst {
		mat4 iview;
		color background;
		int num_trias;
		int num_lights;
		int num_meshes;
		int _a;
		int out_width, out_height;
		float out_ratio;
		int _b;
	} pc;

	vulkan::UniformBuffer *buffer_meshes;

	struct ComputeModeData {
		vulkan::DescriptorPool *pool;
		vulkan::DescriptorSet *dset;
		vulkan::ComputePipeline *pipeline;
	} compute;

	struct RtxModeData {
		vulkan::DescriptorPool *pool;
		vulkan::DescriptorSet *dset;
		vulkan::RayPipeline *pipeline;
		vulkan::AccelerationStructure *tlas = nullptr;
		Array<vulkan::AccelerationStructure*> blas;
		vulkan::UniformBuffer *buffer_cam;
	} rtx;


	shared<Shader> shader_out;
	GraphicsPipeline* pipeline_out = nullptr;
	DescriptorSet *dset_out;
	owned<VertexBuffer> vb_2d;

	Entity *dummy_cam_entity;
	Camera *dummy_cam;
};

#endif
