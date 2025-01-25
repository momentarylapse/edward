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

	Window* window = nullptr;
	vulkan::Instance* instance = nullptr;
	vulkan::DescriptorPool* pool = nullptr;
	vulkan::Device* device = nullptr;
	vulkan::Texture* tex_white = nullptr;
	vulkan::Texture* tex_black = nullptr;
	vulkan::Shader* shader = nullptr;
	vulkan::Shader* shader_lines = nullptr;
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::GraphicsPipeline* pipeline_alpha = nullptr;
	vulkan::GraphicsPipeline* pipeline_lines = nullptr;
	vulkan::VertexBuffer* vb = nullptr;

	vulkan::DescriptorSet* dset = nullptr;
	vulkan::DescriptorSet* dset_lines = nullptr;


	Array<vulkan::VertexBuffer*> line_vbs;
	int num_line_vbs_used = 0;
	vulkan::VertexBuffer* get_line_vb();


	vulkan::Fence* in_flight_fence = nullptr;
	Array<vulkan::Fence*> wait_for_frame_fences;
	vulkan::Semaphore *image_available_semaphore = nullptr, *render_finished_semaphore = nullptr;

	Array<vulkan::CommandBuffer*> command_buffers;
	vulkan::CommandBuffer* current_command_buffer() const;
	vulkan::FrameBuffer* current_frame_buffer() const;

	vulkan::SwapChain *swap_chain = nullptr;
	vulkan::RenderPass* render_pass = nullptr;
	vulkan::DepthBuffer* depth_buffer = nullptr;
	Array<vulkan::FrameBuffer*> frame_buffers;
	int image_index = 0;
	bool framebuffer_resized = true;

};

}

#endif
