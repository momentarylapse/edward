#pragma once

#if HAS_LIB_VULKAN

#include "../vulkan/vulkan.h"

namespace xhui {

class Painter;
class Window;

class ContextVulkan {
public:
	explicit ContextVulkan(Window* window);

	void _create_swap_chain_and_stuff();
	void api_init();
	void rebuild_default_stuff();
	void resize(int w, int h);

	bool start();
	void end();

	Window* window;
	vulkan::Instance* instance = nullptr;
	vulkan::DescriptorPool* pool = nullptr;
	vulkan::Device* device = nullptr;
	vulkan::Texture* tex_white = nullptr;
	vulkan::Texture* tex_black = nullptr;
	vulkan::Shader* shader;
	vulkan::GraphicsPipeline* pipeline;
	vulkan::GraphicsPipeline* pipeline_alpha;
	vulkan::VertexBuffer* vb;

	vulkan::DescriptorSet* dset;


	vulkan::Fence* in_flight_fence;
	Array<vulkan::Fence*> wait_for_frame_fences;
	vulkan::Semaphore *image_available_semaphore, *render_finished_semaphore;

	Array<vulkan::CommandBuffer*> command_buffers;
	vulkan::CommandBuffer* current_command_buffer() const;
	vulkan::FrameBuffer* current_frame_buffer() const;

	vulkan::SwapChain *swap_chain;
	vulkan::RenderPass* render_pass;
	vulkan::DepthBuffer* depth_buffer;
	Array<vulkan::FrameBuffer*> frame_buffers;
	int image_index;
	bool framebuffer_resized;

};

}

#endif
