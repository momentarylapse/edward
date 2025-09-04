#pragma once

#include <lib/base/base.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace ygfx {
	struct DrawingHelperData;
}

namespace xhui {

class Painter;
class Window;

class Context {
public:
	explicit Context(Window* window, ygfx::Context* ctx);
	static Context* create(Window* window);

#if HAS_LIB_VULKAN
	void _create_swap_chain_and_stuff();
#endif
	void rebuild_default_stuff();
	void resize(int w, int h);

	Painter* prepare_draw();
	void begin_draw(Painter* p);
	void end_draw(Painter* p);

	Window* window = nullptr;
	ygfx::Context* context = nullptr;
	ygfx::DrawingHelperData* aux = nullptr;
#if HAS_LIB_VULKAN
	vulkan::Device* device = nullptr;
#endif
	ygfx::Texture* tex_white = nullptr;
	ygfx::Texture* tex_black = nullptr;


#if HAS_LIB_VULKAN
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
#else
#endif
	bool framebuffer_resized = true;
};

}

