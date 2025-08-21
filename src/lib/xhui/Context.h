#pragma once

#include <lib/base/base.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace xhui {

class Painter;
class Window;

class Context {
public:
	explicit Context(Window* window);

#if HAS_LIB_VULKAN
	void _create_swap_chain_and_stuff();
#endif
	void api_init();
	void rebuild_default_stuff();
	void resize(int w, int h);

	bool start();
	void end();

	Window* window = nullptr;
#if HAS_LIB_VULKAN
	vulkan::Instance* instance = nullptr;
	vulkan::DescriptorPool* pool = nullptr;
	vulkan::Device* device = nullptr;
#else
	nix::Context* ctx = nullptr;
#endif
	ygfx::Texture* tex_white = nullptr;
	ygfx::Texture* tex_black = nullptr;
	ygfx::Shader* shader = nullptr;
	ygfx::Shader* shader_lines = nullptr;
#if HAS_LIB_VULKAN
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::GraphicsPipeline* pipeline_alpha = nullptr;
	vulkan::GraphicsPipeline* pipeline_lines = nullptr;
#endif
	ygfx::VertexBuffer* vb = nullptr;


#if HAS_LIB_VULKAN
	vulkan::DescriptorSet* dset = nullptr;
	vulkan::DescriptorSet* dset_lines = nullptr;


	Array<vulkan::VertexBuffer*> line_vbs;
	int num_line_vbs_used = 0;
	Array<vulkan::VertexBuffer*> line_vbs_with_color;
	int num_line_vbs_with_color_used = 0;
	vulkan::VertexBuffer* get_line_vb(bool with_color = false);


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
#endif
	bool framebuffer_resized = true;

};

}

