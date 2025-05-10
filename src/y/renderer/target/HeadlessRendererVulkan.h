//
// Created by michi on 1/12/25.
//

#ifndef HEADLESSRENDERERVULKAN_H
#define HEADLESSRENDERERVULKAN_H

#include "TextureRenderer.h"

#ifdef USING_VULKAN

// TODO "task executor"...
class HeadlessRenderer : public RenderTask {
public:
	vulkan::Device* device;
	CommandBuffer* command_buffer;
	vulkan::Fence* fence;

	owned<TextureRenderer> texture_renderer;

	HeadlessRenderer(vulkan::Device* d, const shared_array<Texture>& tex);
	~HeadlessRenderer() override;

	void render(const RenderParams& params) override;

	RenderParams create_params(const rect& area) const;
};

#endif

#endif //HEADLESSRENDERERVULKAN_H
