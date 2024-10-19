/*
 * WorldRendererVulkanForward.h
 *
 *  Created on: Nov 18, 2021
 *      Author: michi
 */

#pragma once

#include "WorldRendererVulkan.h"
#ifdef USING_VULKAN

class Camera;
class PerformanceMonitor;
struct UBO;
class ShadowRendererVulkan;

class WorldRendererVulkanForward : public WorldRendererVulkan {
public:
	WorldRendererVulkanForward(vulkan::Device *device, Camera *cam);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void render_into_texture(FrameBuffer *fb, Camera *cam, RenderViewDataVK &rvd, const RenderParams& params) override;

	vulkan::Device *device;
};

#endif
