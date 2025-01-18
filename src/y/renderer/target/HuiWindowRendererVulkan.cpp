#if __has_include(<lib/hui/hui.h>)
#include "HuiWindowRenderer.h"
#ifdef USING_VULKAN
#include "TextureRenderer.h"
#include "../../graphics-impl.h"
#include <lib/hui/hui.h>

HuiWindowRenderer::HuiWindowRenderer(vulkan::Instance* instance) : Renderer("hui") {
	device = vulkan::Device::create_simple(instance, nullptr, {"graphics", "anisotropy"});

	command_buffer = new CommandBuffer(device->command_pool);
	fence = new vulkan::Fence(device);

	image.create(MAX_WIDTH, MAX_HEIGHT, Red);
	texture = new Texture(MAX_WIDTH, MAX_HEIGHT, "bgra:i8");
	texture_renderer = new TextureRenderer("tex", {texture});
}

void HuiWindowRenderer::prepare(const RenderParams& p) {
	texture_renderer->children = children;
	texture_renderer->render(p);
}

void HuiWindowRenderer::render_frame(Painter* p) {
	int scale = hui::get_event()->row_target;
	int w = p->width * scale;
	int h = p->height * scale;

	const auto params = create_params(w, h);

	command_buffer->begin();

	prepare(params);

	command_buffer->end();
	device->graphics_queue.submit(command_buffer, {}, {}, fence);
	fence->wait();
	//device->wait_idle();

	texture->read(&image.data[0]);
	//image.mode = Image::Mode::BGRA;
	//image.set_mode(Image::Mode::RGBA);

	float t[4] = {1 / (float)scale, 0, 0, 1 / (float)scale};
	p->set_transform(t, {0,0});
	p->draw_image({0, 0}, &image);
}

RenderParams HuiWindowRenderer::create_params(int w, int h) {
	RenderParams params;
	params.command_buffer = command_buffer;
	params.area = {0, (float)w, 0, (float)h};
	params.render_pass = texture_renderer->render_pass.get();
	params.desired_aspect_ratio = (float)w / (float)h;
	params.frame_buffer = texture_renderer->frame_buffer.get();
	return params;
	//return texture_renderer->create_params({0, (float)w, 0, (float)h});
}

#endif
#endif
