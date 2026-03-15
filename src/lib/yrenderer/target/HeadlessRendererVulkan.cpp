#include "HeadlessRendererVulkan.h"

#ifdef USING_VULKAN
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>

namespace yrenderer {

HeadlessRenderer::HeadlessRenderer(Context* ctx, const shared_array<ygfx::Texture>& tex) : RenderTask(ctx, "headless")
{
	device = ctx->device;
	command_buffer = new ygfx::CommandBuffer(device->command_pool);
	fence = new vulkan::Fence(device);

	texture_renderer = new TextureRenderer(ctx, "tex", tex);
}

HeadlessRenderer::~HeadlessRenderer() = default;

RenderParams HeadlessRenderer::create_params(const rect& area) const {
	auto p = RenderParams::into_texture(texture_renderer->frame_buffer.get(), area.width() / area.height());
	p.area = area;
	p.render_pass = texture_renderer->render_pass.get();
	p.command_buffer = command_buffer;
	return p;
}

void HeadlessRenderer::render(const RenderParams& params) {
	texture_renderer->children = children;

	const auto p = create_params(texture_renderer->frame_buffer->area());
	command_buffer->begin();
	texture_renderer->render(p);
	command_buffer->end();
	device->graphics_queue.submit(command_buffer, {}, {}, fence);
	fence->wait();
	fence->reset();
	//device->wait_idle();
}

}

#endif
