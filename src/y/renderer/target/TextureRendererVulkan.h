#pragma once

#include "../Renderer.h"

#ifdef USING_VULKAN

class TextureRendererVulkan : public Renderer {
public:
	vulkan::Device* device;
	RenderPass* render_pass;
	FrameBuffer* frame_buffer;
	shared<Texture> texture;
	shared<Texture> depth_buffer;
	CommandBuffer* command_buffer;
	vulkan::Fence* fence;

	TextureRendererVulkan(vulkan::Device* d, shared<Texture> tex);
	RenderParams create_params(const rect& area) const;
	void render_frame(const rect& area, float aspect_ratio);
};

#endif
