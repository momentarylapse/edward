#include "TextureRendererVulkan.h"
#ifdef USING_VULKAN
#include "../../graphics-impl.h"

TextureRendererVulkan::TextureRendererVulkan(vulkan::Device* d, shared<Texture> tex) : Renderer("xheadless") {
	device = d;
	texture = tex; //new Texture(width, height, "bgra:i8");
	depth_buffer = new DepthBuffer(texture->width, texture->height, "d:f32", false);
	render_pass = new RenderPass({texture.get(), depth_buffer.get()});
	frame_buffer = new FrameBuffer(render_pass, {texture, depth_buffer});
	command_buffer = new CommandBuffer(device->command_pool);
	fence = new vulkan::Fence(device);
}

RenderParams TextureRendererVulkan::create_params(const rect& area) const {
	auto p = RenderParams::into_texture(frame_buffer, area.width() / area.height());
	p.area = area;
	p.command_buffer = command_buffer;
	p.render_pass = render_pass;
	return p;
}

void TextureRendererVulkan::render_frame(const rect& area, float aspect_ratio) {
	const auto p = create_params(area);
	command_buffer->begin();
	prepare(p);
	command_buffer->begin_render_pass(render_pass, frame_buffer);
	command_buffer->set_viewport(area);
	command_buffer->set_bind_point(vulkan::PipelineBindPoint::GRAPHICS);
	draw(p);
	command_buffer->end_render_pass();
	command_buffer->end();
	device->graphics_queue.submit(command_buffer, {}, {}, fence);
	fence->wait();
	//device->wait_idle();
}

#endif
