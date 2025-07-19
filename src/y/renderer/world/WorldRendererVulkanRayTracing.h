/*
 * WorldRendererVulkanRayTracing.h
 *
 *  Created on: Sep 23, 2022
 *      Author: michi
 */

#pragma once

#include "WorldRenderer.h"
#ifdef USING_VULKAN
#include "../post/ThroughShaderRenderer.h"
#include "../scene/RenderViewData.h"

class Camera;
enum class RaytracingMode;

class WorldRendererVulkanRayTracing : public WorldRenderer {
public:
	WorldRendererVulkanRayTracing(vulkan::Device *device, SceneView& scene_view, int width, int height);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	//void render_into_texture(Camera *cam, RenderViewData &rvd, const RenderParams& params) override;

	RaytracingMode mode;

	vulkan::Device *device;
	RenderViewData rvd;

	vulkan::StorageTexture *offscreen_image;
	//vulkan::Texture *offscreen_image2;
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
		float t_rand;
	} pc;

	struct ComputeModeData {
		vulkan::DescriptorPool *pool;
		vulkan::DescriptorSet *dset;
		vulkan::ComputePipeline *pipeline;
	} compute;

	struct RtxModeData {
		vulkan::DescriptorPool *pool;
		vulkan::DescriptorSet *dset;
		vulkan::RayPipeline *pipeline;
		//vulkan::AccelerationStructure *tlas = nullptr;
		//Array<vulkan::AccelerationStructure*> blas;
		vulkan::UniformBuffer *buffer_cam;
	} rtx;

	owned<ThroughShaderRenderer> out_renderer;
};

#endif
